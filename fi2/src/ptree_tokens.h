
#ifdef __PTREE_TOKEN_MAPDEF
#define __PTREE_TOKEN(a) {Type::a,#a},
#endif
#ifdef __PTREE_TOKEN_ENUMDEF
#define __PTREE_TOKEN(a) a,
#endif

////////////////////////////////////////
__PTREE_TOKEN(unset)
__PTREE_TOKEN(identifier)
__PTREE_TOKEN(openBrace)
__PTREE_TOKEN(error)
__PTREE_TOKEN(whitespace)
__PTREE_TOKEN(colon)
__PTREE_TOKEN(equals)
__PTREE_TOKEN(semicolon)
__PTREE_TOKEN(closeBrace)
__PTREE_TOKEN(eof)
__PTREE_TOKEN(integer)
__PTREE_TOKEN(float_)
__PTREE_TOKEN(string)
//__PTREE_TOKEN(qualifiedName)
__PTREE_TOKEN(comma)
__PTREE_TOKEN(Color)
__PTREE_TOKEN(percentage)
__PTREE_TOKEN(percent)
__PTREE_TOKEN(dimension)
__PTREE_TOKEN(doubleColon)
__PTREE_TOKEN(doubleLess)
__PTREE_TOKEN(doubleMore)
__PTREE_TOKEN(literalLine)
__PTREE_TOKEN(Vector3fList)
__PTREE_TOKEN(at)
__PTREE_TOKEN(dot)
__PTREE_TOKEN(plus)
__PTREE_TOKEN(minus)
__PTREE_TOKEN(multiply)
__PTREE_TOKEN(divide)
__PTREE_TOKEN(openParen)
__PTREE_TOKEN(closeParen)
__PTREE_TOKEN(function)
__PTREE_TOKEN(dollar)
__PTREE_TOKEN(Vector3f)
__PTREE_TOKEN(uMinus)

////////////////////////////////////////

#undef __PTREE_TOKEN_MAPDEF
#undef __PTREE_TOKEN_ENUMDEF
#undef __PTREE_TOKEN
 