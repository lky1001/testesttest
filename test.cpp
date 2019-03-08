void love_tatto::signin(name account, uint64_t type, std::string username, std::string profile, std::string profileurl)
{
    require_auth(account);

    user_index user_table(_code, _code);
    auto user_name_index = user_index.get_index<"byowner"_n>();
    auto iterator = user_name_index.find(account.value);

    if (iterator != user_table.end())
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
