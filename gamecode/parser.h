
#ifndef GAMECODE_PARSER_H
#define GAMECODE_PARSER_H
#include <vector>


class Monstertype;
class ObjectType;

void monster_parser();
void object_parser();

extern std::vector<Monstertype> monster_types;
extern std::vector<ObjectType> object_types;

#endif //GAMECODE_PARSER_H
