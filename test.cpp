#include "love_tatto.hpp"

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
        row.created = cur_time;
    });
}

void love_tattoo::matchcouple(name from, name to)
{
    require_auth(from);
    eosio_assert(is_account(to), "to account does not exist");

    // from 유저의 커플 매칭 내역 찾기
    couple_index couple_table(_code, from);
    auto from_user_index = couple_table.get_index<"byfromuser"_n>();
    auto from_user_iterator = from_user_index.find(from.value);

    // 신청 내역이 있을 경우 만료 여부 체크
    if (from_user_iterator != couple_index.end())
    {
        uint64_t cur_time = current_time();

        // 만료 새로 추가
        if (from_user_iterator->expired <= cur_time)
        {
            couple_table.emplace(from, [&](auto &row) {
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
        if (to_user_iterator != couple_index.end())
        {
            // 내가 매칭 요청 상대와 나를 매칭 요청한 상대가 같을 경우 매칭 완료
            if (to_user_iterator->from_user.value == to.value)
            {
                uint64_t cur_time = current_time();

                user_index user_table(_code, _code);
                auto from_user_index = couple_table.get_index<"byowner"_n>();
                auto from_user_iterator = from_user_index.find(from.value);

                if (from_user_iterator != user_index.end())
                {
                    user_table.modify(from_user_iterator, _code, [&](auto &row) {
                        row.match_user = to;
                        row.matching_date = cur_time;
                        row.updated = cur_time;
                    });
                }
                else
                {
                    eosio_assert(false, "from user not found.");
                }

                auto to_user_index = couple_table.get_index<"byowner"_n>();
                auto to_user_iterator = to_user_index.find(to.value);

                if (to_user_iterator != user_index.end())
                {
                    user_table.modify(to_user_iterator, _code, [&](auto &row) {
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
                couple_table.emplace(from, [&](auto &row) {
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
            couple_table.emplace(from, [&](auto &row) {
                row.id = couple_table.available_primary_key();
                row.from_user = from;
                row.to_user = to;
                row.expired = cur_time + cur_time + ((uint64_t)(1LLU * 86400 * 1000000));
                row.created = cur_time;
            });
        }
    }
}

void love_tattoo::write(name creator, uint64_t posttype, std::string contents, std::string attach_url)
{
}
