#pragma once

#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>

using namespace eosio;

static const std::string USER_STATE_ACTIVE = "active";
static const std::string USER_STATE_BLOCK = "block";

CONTRACT love_tattoo : public contract
{
  public:
    love_tattoo(name self, name code, datastream<const char *> ds) : contract(self, code, ds){};
    ACTION signin(name account, uint64_t type, std::string username, std::string profile, std::string profileurl);
    ACTION matchcouple(name from, name to);
    ACTION write(name creator, uint64_t posttype, uint64_t missionid, std::string contents, std::string attachurl);
    ACTION vote(name voter, uint64_t postid, uint64_t islike);
    ACTION newmission(std::string text, std::string imageurl, uint64_t startdate, uint64_t enddate);
    ACTION block(name user, std::string state, std::string reason);

    // token
    ACTION create(name issuer, asset maximum_supply);
    ACTION issue(name to, asset quantity, std::string memo);
    ACTION transfer(name from, name to, asset quantity, std::string memo);

    inline asset get_supply(symbol_code sym) const;
    inline asset get_balance(name owner, symbol_code sym) const;

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

        std::string state;
        std::string reason;

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
        uint64_t mission_id;

        std::string contents;
        std::string attach_url;

        uint64_t created;
        uint64_t updated;

        uint64_t primary_key() const { return id; }
        uint64_t by_creator() const { return creator.value; }
    };

    TABLE mission
    {
        uint64_t id;
        name creator;

        std::string text;
        std::string image_url;

        uint64_t start_date;
        uint64_t end_date;

        uint64_t created;
        uint64_t updated;

        uint64_t primary_key() const { return id; }
    };

    TABLE votes
    {
        uint64_t id;

        uint64_t post_id;
        name voter;

        // 0 싥어요, 1 좋아요
        uint64_t is_like;

        uint64_t created;

        uint64_t primary_key() const { return id; }
        uint64_t by_post_id() const { return post_id; }
        uint64_t by_voter() const { return voter.value; }
    };

    typedef multi_index<"users"_n, user, indexed_by<"byowner"_n, const_mem_fun<user, uint64_t, &user::by_owner>>, indexed_by<"bymatchuser"_n, const_mem_fun<user, uint64_t, &user::by_match_user>>> user_index;
    typedef multi_index<"couples"_n, couple, indexed_by<"byfromuser"_n, const_mem_fun<couple, uint64_t, &couple::by_from_user>>, indexed_by<"bytouser"_n, const_mem_fun<couple, uint64_t, &couple::by_to_user>>> couple_index;
    typedef multi_index<"posts"_n, post, indexed_by<"bycreator"_n, const_mem_fun<post, uint64_t, &post::by_creator>>> post_index;
    typedef multi_index<"votes"_n, votes, indexed_by<"byvoter"_n, const_mem_fun<votes, uint64_t, &votes::by_voter>>, indexed_by<"bypostid"_n, const_mem_fun<votes, uint64_t, &votes::by_post_id>>> votes_index;
    typedef multi_index<"missions"_n, mission> mission_index;

    // token
    TABLE account
    {
        asset balance;

        uint64_t primary_key() const { return balance.symbol.code().raw(); }
    };

    TABLE currency_stats
    {
        asset supply;
        asset max_supply;
        name issuer;

        uint64_t primary_key() const { return supply.symbol.code().raw(); }
    };

    typedef eosio::multi_index<"accounts"_n, account> accounts;
    typedef eosio::multi_index<"stat"_n, currency_stats> stats;

    void sub_balance(name owner, asset value);
    void add_balance(name owner, asset value, name ram_payer);

  public:
    struct transfer_args
    {
        name from;
        name to;
        asset quantity;
        std::string memo;
    };
};

asset love_tattoo::get_supply(symbol_code sym) const
{
    stats statstable(_self, sym.raw());
    const auto &st = statstable.get(sym.raw());
    return st.supply;
}

asset love_tattoo::get_balance(name owner, symbol_code sym) const
{
    accounts accountstable(_self, owner.value);
    const auto &ac = accountstable.get(sym.raw());
    return ac.balance;
}
