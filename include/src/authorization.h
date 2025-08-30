#ifndef AUTHORIZATION_H
#define AUTHORIZATION_H

#include <string>
#include <iostream>

bool checker_authorization();
bool authorization(std::string login, std::string password);
std::string registration(std::string login, std::string email, std::string password);

#endif // AUTHORIZATION_H