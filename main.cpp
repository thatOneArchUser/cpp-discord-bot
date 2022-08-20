#include <algorithm>
#include <dpp/dpp.h>
#include <dpp/user.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <time.h>
#include <stdlib.h>
#include <cmath>
#include <sys/stat.h>

#define BASE 10

using namespace std;
using json = nlohmann::json;

typedef unsigned long dword;
typedef unsigned long long qword;
typedef vector<dpp::snowflake> snowflake;

ifstream f("settings.json");
const json settings = json::parse(f);
const qword id = settings["id"];

inline bool isNumber(const string &s)
{
    string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

inline bool fileExists(const string &name) {
    struct stat buff;
    return (stat(name.c_str(), &buff) == 0);
}

int main() {
    dpp::cluster bot(settings["token"], dpp::i_default_intents | dpp::i_message_content);

    bot.on_log(dpp::utility::cout_logger());
    
    dpp::commandhandler command_handler(&bot);
    command_handler.add_prefix(".");
    
    string files[] = {"money.json", "leveling.json"};
    for (string i : files) {
        if (!fileExists(i)) {
            ofstream f(i);
            f << "{ }";
            f.close();
        }
    }

    bot.on_message_create([&bot](const dpp::message_create_t &event) {
        if (!event.msg.author.is_bot()) {
            ifstream f("./leveling.json");
            json leveling = json::parse(f);
            if (!leveling.contains(to_string(event.msg.author.id))) {
                leveling[to_string(event.msg.author.id)] = {0, 1};
            }
            qword xp = leveling[to_string(event.msg.author.id)][0];
            dword level = leveling[to_string(event.msg.author.id)][1];
            qword xpreq = 0;
            leveling[to_string(event.msg.author.id)][0] = leveling[to_string(event.msg.author.id)][0].get<qword>() + 1;
            for (dword i = 0; i < level; ++i) {
                xpreq = round(xpreq*1.2 + 15);
            }
            if (xpreq <= xp) {
                leveling[to_string(event.msg.author.id)][1] = leveling[to_string(event.msg.author.id)][1].get<dword>() + 1;
                leveling[to_string(event.msg.author.id)][0] = 0;
                bot.message_create(event.msg.author.get_mention() + " you just leveled up to level " + to_string(leveling[to_string(event.msg.author.id)][1]));
            }
            ofstream file("./leveling.json");
            file << leveling;
            file.close();

            /*                 commands with no parameters                    */
            if (event.msg.content == ".shutdown" && event.msg.author.id == settings["id"]) {
                exit(0);
            }

            if (event.msg.content == ".beg") {
                ifstream f("./money.json");
                json data = json::parse(f);
                srand(time(NULL));
                unsigned short money = rand() % 6969 + 1;
                if (!data.contains(to_string(event.msg.author.id))) {
                    data[to_string(event.msg.author.id)] = {0, 0};
                }
                data[to_string(event.msg.author.id)][0] = data[to_string(event.msg.author.id)][0].get<dword>() + money;
                ofstream file("./money.json");
                file << data;
                file.close();
                bot.message_create(dpp::message(event.msg.channel_id, "You earned " + to_string(money) + " coins"));
            }
        }
    });


    /*            commands with parameters                   */
    bot.on_ready([&command_handler](const dpp::ready_t &event) {
        command_handler.add_command(
            "balance",
            {
                {"user", dpp::param_info(dpp::pt_user, true, "q")}
            },
            [&command_handler](const string &command, const dpp::parameter_list_t &parameters, dpp::command_source src) {
                ifstream f("./money.json");
                json data = json::parse(f);
                dpp::user user;
                try {
                    dpp::resolved_user u = get<dpp::resolved_user>(parameters[0].second);
                    user = u.user;
                } catch (...) {
                    user = src.issuer;
                }
                if (!data.contains(to_string(user.id))) {
                    data[to_string(user.id)] = {0, 0};
                }
                dword wallet = data[to_string(user.id)][0];
                dword bank = data[to_string(user.id)][1];
                dpp::embed embed = dpp::embed().
                    set_color(dpp::colors::cyan).
                    set_title(user.username + "'s balance").
                    set_description("Wallet: " + to_string(wallet) + "\nBank: " + to_string(bank));
                command_handler.reply(dpp::message(src.channel_id, embed), src);
            },
            "command"
        );

        command_handler.add_command(
            "level",
            {
                {"user", dpp::param_info(dpp::pt_user, true, "q")}
            },
            [&command_handler](const string &command, const dpp::parameter_list_t &parameters, dpp::command_source src) {
                ifstream f("./leveling.json");
                json data = json::parse(f);
                dpp::user user;
                try {
                    dpp::resolved_user u = get<dpp::resolved_user>(parameters[0].second);
                    user = u.user;
                } catch (...) {
                    user = src.issuer;
                }
                if (!data.contains(to_string(user.id))) {
                    data[to_string(user.id)] = {0, 1};
                }
                dword level = data[to_string(user.id)][1];
                qword xp = data[to_string(user.id)][0];
                qword xpreq = 0;
                for (dword i = 0; i < level; ++i) {
                    xpreq = round(xpreq*1.2 + 15);
                }
                dpp::embed embed = dpp::embed().
                    set_color(dpp::colors::cyan).
                    set_title(user.username + "'s level").
                    set_description("Level: " + to_string(level) + "\nxp: " + to_string(xp) + "/" + to_string(xpreq));
                command_handler.reply(dpp::message(src.channel_id, embed), src);
            }
        );

        command_handler.add_command(
            "deposit",
            {
                {"amount", dpp::param_info(dpp::pt_string, false, "amount of coins")}
            },
            [&command_handler](const string &command, const dpp::parameter_list_t &parameters, dpp::command_source src) {
                const string a = get<string>(parameters[0].second);
                char** __restrict stopstring;
                ifstream f("./money.json");
                json data = json::parse(f);
                dword deposited;
                if (a == "max" || a == "all") {
                    dword total = data[to_string(src.issuer.id)][0];
                    deposited = total;
                    data[to_string(src.issuer.id)][1] = data[to_string(src.issuer.id)][1].get<dword>() + total;
                    data[to_string(src.issuer.id)][0] = 0;
                } else if (isNumber(a)) {
                    dword amount;
                    amount = strtoul(a.c_str(), stopstring, BASE);
                    if (amount > data[to_string(src.issuer.id)][0]) {
                        command_handler.reply(dpp::message(src.channel_id, "You don't have that many coins in your wallet"), src);
                        return;
                    }
                    deposited = amount;
                    data[to_string(src.issuer.id)][1] = data[to_string(src.issuer.id)][1].get<dword>() + amount;
                    data[to_string(src.issuer.id)][0] = data[to_string(src.issuer.id)][0].get<dword>() - amount;
                } else {
                    command_handler.reply(dpp::message(src.channel_id, "Invalid argument"), src);
                    return;
                }
                ofstream file("./money.json");
                file << data;
                command_handler.reply(dpp::message(src.channel_id, "You deposited " + to_string(deposited) + " coins"), src);
            }
        );

        command_handler.add_command(
            "withdraw",
            {
                {"amount", dpp::param_info(dpp::pt_string, false, "amount of coins")}
            },
            [&command_handler](const string &command, const dpp::parameter_list_t &parameters, dpp::command_source src) {
                const string a = get<string>(parameters[0].second);
                char** __restrict stopstring;
                ifstream f("./money.json");
                json data = json::parse(f);
                dword withdrawn;
                if (a == "max" || a == "all") {
                    dword total = data[to_string(src.issuer.id)][1];
                    withdrawn = total;
                    data[to_string(src.issuer.id)][0] = data[to_string(src.issuer.id)][0].get<dword>() + total;
                    data[to_string(src.issuer.id)][1] = 0;
                } else if (isNumber(a)) {
                    dword amount;
                    amount = strtoul(a.c_str(), stopstring, BASE);
                    if (amount > data[to_string(src.issuer.id)][1]) {
                        command_handler.reply(dpp::message(src.channel_id, "You don't have that many coins in your bank"), src);
                        return;
                    }
                    withdrawn = amount;
                    data[to_string(src.issuer.id)][0] = data[to_string(src.issuer.id)][0].get<dword>() + amount;
                    data[to_string(src.issuer.id)][1] = data[to_string(src.issuer.id)][1].get<dword>() - amount;
                } else {
                    command_handler.reply(dpp::message(src.channel_id, "Invalid argument"), src);
                    return;
                }
                ofstream file("./money.json");
                file << data;
                command_handler.reply(dpp::message(src.channel_id, "You withdrawn " + to_string(withdrawn) + " coins"), src);
            }
        );
        command_handler.register_commands();
    });

    bot.start(dpp::st_wait);
}
