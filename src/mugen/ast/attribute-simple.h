#ifndef _paintown_ast_attribute_simple_h_
#define _paintown_ast_attribute_simple_h_

#include "attribute.h"
#include "identifier.h"
#include "Value.h"
#include <sstream>

namespace Ast{

class AttributeSimple: public Attribute {
public:
    
    AttributeSimple(const Identifier * name, const Value * value):
    Attribute(Simple),
    name(name),
    value(value){
    }
    
    AttributeSimple(const Identifier * name):
    Attribute(Simple),
    name(name),
    value(0){
    }

    bool operator==(const std::string & str) const {
        return *name == str;
    }

    const AttributeSimple & operator>>(std::string & str) const {
        if (value != 0){
            *value >> str;
        }
        return *this;
    }

    virtual bool referenced(const void * value) const {
        return Attribute::referenced(value) ||
               name->referenced(value) ||
               (this->value != 0 && this->value->referenced(value));
    }

    std::string toString(){
        std::ostringstream out;
        if (value != 0){
            out << name->toString() << " = " << value->toString();
        } else {
            out << name->toString() << " = <none>";
        }

        return out.str();
    }

    virtual ~AttributeSimple(){
        delete name;
        delete value;
    }

protected:
    const Identifier * name;
    const Value * value;
};

}

#endif
