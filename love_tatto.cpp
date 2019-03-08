#include "love_tattoo.hpp"

void love_tattoo::signin(name account, uint64_t type, std::string username, std::string profile, std::string profileurl)
{
    require_auth(account);

    user_index user_table(_code, _code.value);
    auto user_name_index = user_table.get_index<"byowner"_n>();
    auto iterator = user_name_index.find(account.value);

    if (iterator != user_name_index.end())
    {
        eosio_assert(false, "account already exist.");
    }

    uint64_t cur_time = current_time();

    user_table.emplace(_code, [&](auto &row) {
        row.id = user_table.available_primary_key();
        row.owner = account;
        row.type = type;
        row.user_name = username;
        row.profile = profile;
        row.profile_url = profileurl;
        row.state = USER_STATE_ACTIVE;
        row.created = cur_time;
    });
}

void love_tattoo::matchcouple(name from, name to)
{
    require_auth(from);
    eosio_assert(is_account(to), "to account does not exist");

    // from 유저의 커플 매칭 내역 찾기
    couple_index couple_table(_code, from.value);
    auto from_user_index = couple_table.get_index<"byfromuser"_n>();
    auto from_user_iterator = from_user_index.find(from.value);

    // 신청 내역이 있을 경우 만료 여부 체크
    if (from_user_iterator != from_user_index.end())
    {
        uint64_t cur_time = current_time();

        // 만료 새로 추가
        if (from_user_iterator->expired <= cur_time)
        {
            couple_table.emplace(_code, [&](auto &row) {
                row.id = couple_table.available_primary_key();
                row.from_user = from;
                row.to_user = to;
                row.expired = cur_time + cur_time + ((uint64_t)(1LLU * 86400 * 1000000));
                row.created = cur_time;
            });
        }
        else
        {
            // 이미 신청 내역이 있어서 오류
            eosio_assert(false, "matching already exist.");
        }
    }
    else
    {
        // 신청 내역이 없을 경우 상대방으로부터 매칭 요청 내역이 있는지 확인
        auto to_user_index = couple_table.get_index<"bytouser"_n>();
        auto to_user_iterator = to_user_index.find(from.value);

        // 매칭 요청이 있을 경우
        if (to_user_iterator != to_user_index.end())
        {
            // 내가 매칭 요청 상대와 나를 매칭 요청한 상대가 같을 경우 매칭 완료
            if (to_user_iterator->from_user.value == to.value)
            {
                uint64_t cur_time = current_time();

                user_index user_table(_code, _code.value);
                auto from_user_index = user_table.get_index<"byowner"_n>();
                auto from_user_iterator = from_user_index.find(from.value);

                if (from_user_iterator != from_user_index.end())
                {
                    from_user_index.modify(from_user_iterator, _code, [&](auto &row) {
                        row.match_user = to;
                        row.matching_date = cur_time;
                        row.updated = cur_time;
                    });
                }
                else
                {
                    eosio_assert(false, "from user not found.");
                }

                auto to_user_index = user_table.get_index<"byowner"_n>();
                auto to_user_iterator = to_user_index.find(to.value);

                if (to_user_iterator != to_user_index.end())
                {
                    to_user_index.modify(to_user_iterator, _code, [&](auto &row) {
                        row.match_user = from;
                        row.matching_date = cur_time;
                        row.updated = cur_time;
                    });
                }
                else
                {
                    eosio_assert(false, "to user not found.");
                }
            }
            else
            {
                // 그렇지 않을 경우 신규 추가
                uint64_t cur_time = current_time();
                // 신규 추가
                couple_table.emplace(_code, [&](auto &row) {
                    row.from_user = from;
                    row.to_user = to;
                    row.expired = cur_time + cur_time + ((uint64_t)(1LLU * 86400 * 1000000));
                    row.created = cur_time;
                });
            }
        }
        else
        {
            uint64_t cur_time = current_time();
            // 신규 추가
            couple_table.emplace(_code, [&](auto &row) {
                row.id = couple_table.available_primary_key();
                row.from_user = from;
                row.to_user = to;
                row.expired = cur_time + cur_time + ((uint64_t)(1LLU * 86400 * 1000000));
                row.created = cur_time;
            });
        }
    }
}

void love_tattoo::write(name creator, uint64_t posttype, uint64_t missionid, std::string contents, std::string attachurl)
{
    eosio_assert(is_account(creator), "creator does not exist");

    user_index user_table(_code, _code.value);
    auto user_name_index = user_table.get_index<"byowner"_n>();
    auto iterator = user_name_index.find(creator.value);

    if (iterator != user_name_index.end())
    {
        uint64_t cur_time = current_time();

        post_index post_table(_code, _code.value);

        post_table.emplace(_code, [&](auto &row) {
            row.id = post_table.available_primary_key();
            row.creator = creator;
            row.post_type = posttype;
            row.mission_id = missionid;
            row.contents = contents;
            row.attach_url = attachurl;
            row.created = cur_time;
        });
    }
    else
    {
        eosio_assert(false, "user not found.");
    }
}

void love_tattoo::vote(name votor, uint64_t postid, uint64_t islike)
{
    require_auth(voter);
    
    vote_index vote_table(_code, _code.value);
    auto by_voter_index = vote_table.get_index<"byvoter"_n>();
    auto iterator = by_voter_index.find(votor.value);

    //보팅은 하나의 포스트에 한번만 할 수 있음
    if (iterator == by_voter_index.end())
    {
        uint64_t cur_time = current_time();
        
        vote_table.emplace(_code,[&](auto &row) {
            row.id = vote_table.available_primary_key();
            row.post_id = postid;
            row.voter = votor;
            row.is_like = islike;
            row.created = cur_time;
        });
    }else{
        //보팅이 했었다면 예외
        eosio_assert(false, "you already made a vote.");
    }
}

void love_tattoo::newmission(std::string text, std::string imageurl, uint64_t startdate, uint64_t enddate)
{
    require_auth(_code);

    mission_index mission_table(_code, _code.value);

    uint64_t cur_time = current_time();

    mission_table.emplace(_code, [&](auto &row) {
        row.id = mission_table.available_primary_key();
        row.creator = _code;
        row.text = text;
        row.image_url = imageurl;
        row.start_date = startdate;
        row.end_date = enddate;
        row.created = cur_time;
    });
}

void love_tattoo::block(name user, std::string state, std::string reason)
{
    require_auth(_code);

    user_index user_table(_code, _code.value);
    auto user_name_index = user_table.get_index<"byowner"_n>();
    auto iterator = user_name_index.find(user.value);

    if (iterator != user_name_index.end())
    {
        eosio_assert(false, "account already exist.");
    }

    uint64_t cur_time = current_time();

    user_name_index.modify(iterator, _code, [&](auto &row) {
        row.state = state;
        row.reason = reason;
        row.updated = cur_time;
    });
}

// token

void love_tattoo::create(name issuer, eosio::asset maximum_supply)
{
    require_auth(_self);

    auto sym = maximum_supply.symbol;
    eosio_assert(sym.is_valid(), "invalid symbol name");
    eosio_assert(maximum_supply.is_valid(), "invalid supply");
    eosio_assert(maximum_supply.amount > 0, "max-supply must be positive");

    stats statstable(_self, sym.code().raw());
    auto existing = statstable.find(sym.code().raw());
    eosio_assert(existing == statstable.end(), "token with symbol already exists");

    statstable.emplace(_self, [&](auto &s) {
        s.supply.symbol = maximum_supply.symbol;
        s.max_supply = maximum_supply;
        s.issuer = issuer;
    });
}

void love_tattoo::issue(name to, eosio::asset quantity, std::string memo)
{
    auto sym = quantity.symbol;
    eosio_assert(sym.is_valid(), "invalid symbol name");
    eosio_assert(memo.size() <= 256, "memo has more than 256 bytes");

    auto sym_name = sym.code().raw();
    stats statstable(_self, sym_name);
    auto existing = statstable.find(sym_name);
    eosio_assert(existing != statstable.end(), "token with symbol does not exist, create token before issue");
    const auto &st = *existing;

    require_auth(st.issuer);
    eosio_assert(quantity.is_valid(), "invalid quantity");
    eosio_assert(quantity.amount > 0, "must issue positive quantity");

    eosio_assert(quantity.symbol == st.supply.symbol, "symbol precision mismatch");
    eosio_assert(quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");

    statstable.modify(st, _self, [&](auto &s) {
        s.supply += quantity;
    });

    add_balance(st.issuer, quantity, st.issuer);

    if (to != st.issuer)
    {
        SEND_INLINE_ACTION(*this, transfer, {st.issuer, "active"_n}, {st.issuer, to, quantity, memo});
    }
}

void love_tattoo::transfer(name from, name to, eosio::asset quantity, std::string memo)
{
    eosio_assert(from != to, "cannot transfer to self");
    require_auth(from);
    eosio_assert(is_account(to), "to account does not exist");
    auto sym = quantity.symbol.code().raw();
    stats statstable(_self, sym);
    const auto &st = statstable.get(sym);

    require_recipient(from);
    require_recipient(to);

    eosio_assert(quantity.is_valid(), "invalid quantity");
    eosio_assert(quantity.amount > 0, "must transfer positive quantity");
    eosio_assert(quantity.symbol == st.supply.symbol, "symbol precision mismatch");
    eosio_assert(memo.size() <= 256, "memo has more than 256 bytes");

    sub_balance(from, quantity);
    add_balance(to, quantity, from);
}

void love_tattoo::sub_balance(name owner, eosio::asset value)
{
    accounts from_acnts(_self, owner.value);

    const auto &from = from_acnts.get(value.symbol.code().raw(), "no balance object found");
    eosio_assert(from.balance.amount >= value.amount, "overdrawn balance");

    if (from.balance.amount == value.amount)
    {
        from_acnts.erase(from);
    }
    else
    {
        from_acnts.modify(from, owner, [&](auto &a) {
            a.balance -= value;
        });
    }
}

void love_tattoo::add_balance(name owner, eosio::asset value, name ram_payer)
{
    accounts to_acnts(_self, owner.value);
    auto to = to_acnts.find(value.symbol.code().raw());
    if (to == to_acnts.end())
    {
        to_acnts.emplace(ram_payer, [&](auto &a) {
            a.balance = value;
        });
    }
    else
    {
        to_acnts.modify(to, owner, [&](auto &a) {
            a.balance += value;
        });
    }
}

EOSIO_DISPATCH(love_tattoo, (signin)(matchcouple)(write)(vote)(newmission)(block)(create)(issue)(transfer))
