//
// Created by pravl on 30.04.2025.
//

#include "AuthManager.h"
#include "Services.h"
#include "Database.h"
#include <iostream>
#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>
#ifdef _WIN32
#include <windows.h>
#endif

static bool inputDate(const std::string& prompt, std::tm& dt) {
    std::string s;
    std::cout << prompt << " (YYYY-MM-DD): ";
    std::cin >> s;
    std::istringstream ss(s);
    ss >> std::get_time(&dt, "%Y-%m-%d");
    if (ss.fail()) {
        std::cout << "Неверный формат даты.\n";
        return false;
    }
    return true;
}

int main() {
#ifdef _WIN32
    // Перевод консоли на UTF-8
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    if (!Database::instance().open("../DataBase/dataBase.db")) {
        std::cerr << "Ошибка открытия БД\n"; return 1;
    }

    AuthManager auth;
    std::string user, pass;
    std::cout << "Login: "; std::cin >> user;
    std::cout << "Password: "; std::cin >> pass;
    if (!auth.login(user, pass)) {
        std::cout << "Аутентификация не удалась\n";
        return 0;
    }

    FleetService svc;
    bool isMgr = auth.isManager();
    int cmd = 0;
    while (true) {
        std::cout << "\n=== Меню ===\n";
        if (isMgr) {
            std::cout << "1. Рейсы траулера за период\n"
                      << "2. Улов по банке\n"
                      << "3. Банка с макс низкого качества\n"
                      << "4. Траулер с наибольшим уловом\n"
                      << "5. Пенсионеры на дату\n"
                      << "6. Начислить премии (все)\n"
                      << "7. Начислить премию члену\n"
                      << "0. Выход\n> ";
        } else {
            std::cout << "1. Мои данные\n"
                      << "0. Выход\n> ";
        }
        std::cin >> cmd;
        if (cmd == 0) break;

        if (!isMgr) {
            if (cmd == 1) {
                auto u = auth.currentUser().value();
                std::cout << "ID: " << u.id << "\n"
                          << "Фамилия: " << u.lastName << "\n"
                          << "Должность: " << u.role << "\n"
                          << "Дата приёма: " << std::put_time(&u.hireDate, "%Y-%m-%d") << "\n"
                          << "Год рождения: " << u.birthYear << "\n";
            }
            continue;
        }

        switch (cmd) {
            case 1: {
                int tid; std::tm d1{}, d2{};
                std::cout<<"ID траулера: "; std::cin>>tid;
                if (!inputDate("С", d1) || !inputDate("По", d2)) break;
                auto vlist = svc.getVoyagesByTrawler(tid,d1,d2);
                for (auto& [v, total]: vlist) {
                    std::cout << "Рейс " << v.id
                              << ": " << std::put_time(&v.departureDate,"%Y-%m-%d")
                              << " - " << std::put_time(&v.returnDate,"%Y-%m-%d")
                              << ", улов=" << total << " кг\n";
                }
                break;
            }
            case 2: {
                int bid; std::cout<<"ID банки: "; std::cin>>bid;
                auto clist = svc.getCatchByBank(bid);
                for (auto& [fish, qty]: clist)
                    std::cout<<fish<<": "<<qty<<" кг\n";
                break;
            }
            case 3: {
                auto lv = svc.getMaxLowQualityBankVoyages();
                for (auto& l: lv)
                    std::cout<<"Рейс "<<l.v.id<<" траулер "<<l.trawlerName
                             <<" "<<std::put_time(&l.v.departureDate,"%Y-%m-%d")
                             <<" - "<<std::put_time(&l.v.returnDate,"%Y-%m-%d")<<"\n";
                break;
            }
            case 4: {
                auto tb = svc.getTopTrawlerInfo();
                for (auto& c: tb)
                    std::cout<<"Капитан "<<c.captainLastName
                            <<" банка "<<c.bank.name<<" рейс "<<c.v.id
                             <<" "<<std::put_time(&c.v.departureDate,"%Y-%m-%d")
                             <<"-"<<std::put_time(&c.v.returnDate,"%Y-%m-%d")<<"\n";
                break;
            }
            case 5: {
                std::tm dt{}; if (!inputDate("Дата", dt)) break;
                auto pens = svc.getPensioners(dt);
                for (auto& p: pens)
                    std::cout<<p.lastName<<" ("<<p.role<<") год "<<p.birthYear<<"\n";
                break;
            }
            case 6: {
                std::tm s{}, e{}; double plan, price;
                if (!inputDate("С", s) || !inputDate("По", e)) break;
                std::cout<<"План (кг): "; std::cin>>plan;
                std::cout<<"Цена за кг: "; std::cin>>price;
                bool ok = svc.awardBonuses(s,e,plan,price);
                std::cout<<(ok?"Премии начислены\n":"Ошибка\n");
                break;
            }
            case 7: {
                int cid; std::tm s{}, e{}; double plan, price;
                std::cout<<"ID члена: "; std::cin>>cid;
                if (!inputDate("С", s) || !inputDate("По", e)) break;
                std::cout<<"План (кг): "; std::cin>>plan;
                std::cout<<"Цена за кг: "; std::cin>>price;
                bool ok = svc.awardBonusToMember(cid,s,e,plan,price);
                std::cout<<(ok?"Премия начислена\n":"Ошибка\n");
                break;
            }
            default:
                std::cout<<"Неизвестная команда\n";
        }
    }

    Database::instance().close();
    return 0;
}
