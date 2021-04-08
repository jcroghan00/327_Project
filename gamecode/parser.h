
#ifndef GAMECODE_PARSER_H
#define GAMECODE_PARSER_H
#include <vector>

class Monstertype;

void monster_parser();
void object_parser();

extern std::vector<Monstertype> monster_types;

#endif //GAMECODE_PARSER_H
