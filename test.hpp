#pragma once

#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>

using namespace eosio;

CONTRACT love_tattoo : public contract
{
  public:
    love_tattoo(name self, name code, datastream<const char *> ds) : contract(self, code, ds){};
    ACTION signin(name account, uint64_t type, std::string username, std::string profile, std::string profileurl);
    ACTION matchcouple(name from, name to);
    ACTION write(name creator, uint64_t posttype, std::string contents, std::string attach_url);

    // token
    // ACTION create(name issuer, asset maximum_supply);
    // ACTION issue(name to, asset quantity, string memo);
    // ACTION transfer(name from, name to, asset quantity, string memo);
    // static asset get_supply(symbol_code sym) const;
    // static asset get_balance(name owner, symbol_code sym) const;

  private:
    TABLE user
    {
        uint64_t id;
        name owner;
        // 0 커플, 1 큐피트
        uint64_t type;
        std::string user_name;
        std::string profile;
        std::string profile_url;

        name match_user;
        uint64_t matching_date;

        std::vector<name> voters;

        uint64_t created;
        uint64_t updated;

        uint64_t primary_key() const { return id; }
        uint64_t by_owner() const { return owner.value; }
        uint64_t by_match_user() const { return match_user.value; }
    };

    TABLE couple
    {
        uint64_t id;

        name from_user;
        name to_user;

        uint64_t expired;

        uint64_t created;
        uint64_t updated;

        uint64_t primary_key() const { return id; }
        uint64_t by_from_user() const { return from_user.value; }
        uint64_t by_to_user() const { return to_user.value; }
    };

    TABLE post
    {
        uint64_t id;
        name creator;

        // 0 단순기록, 1 미션인증
        uint64_t post_type;

        std::string contents;
        std::string attach_url;

        uint64_t created;
        uint64_t updated;

        uint64_t primary_key() const { return id; }
        uint64_t by_creator() const { return creator.value; }
    };

    TABLE vote
    {
        uint64_t id;
        name votor;

        // 0 싥어요, 1 좋아요
        uint64_t is_like;

        uint64_t created;

        uint64_t primary_key() const { return id; }
        uint64_t by_votor() const { return votor.value; }
    };

    typedef multi_index<"users"_n, user, indexed_by<"byowner"_n, const_mem_fun<user, uint64_t, &user::by_owner>>, indexed_by<"bymatchuser"_n, const_mem_fun<user, uint64_t, &user::by_match_user>>> user_index;
    typedef multi_index<"couple"_n, user, indexed_by<"byfromuser"_n, const_mem_fun<couple, uint64_t, &user::by_from_user>>, indexed_by<"bytouser"_n, const_mem_fun<couple, uint64_t, &user::by_to_user>>> couple_index;

    // token
    //     struct account
    //     {
    //         asset balance;

    //         uint64_t primary_key() const { return balance.symbol.name(); }
    //     };

    //     struct currency_stats
    //     {
    //         asset supply;
    //         asset max_supply;
    //         name issuer;

    //         uint64_t primary_key() const { return supply.symbol.name(); }
    //     };

    //     typedef eosio::multi_index<N(accounts), account> accounts;
    //     typedef eosio::multi_index<N(stat), currency_stats> stats;

    //     void sub_balance(name owner, asset value);
    //     void add_balance(name owner, asset value, name ram_payer);

    //   public:
    //     struct transfer_args
    //     {
    //         name from;
    //         name to;
    //         asset quantity;
    //         string memo;
    //     };
};

// asset token::get_supply(symbol_name sym) const
// {
//     stats statstable(_self, sym);
//     const auto &st = statstable.get(sym);
//     return st.supply;
// }

// asset token::get_balance(name owner, symbol_name sym) const
// {
//     accounts accountstable(_self, owner);
//     const auto &ac = accountstable.get(sym);
//     return ac.balance;
// }
