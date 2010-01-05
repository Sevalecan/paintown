#include "evaluator.h"
#include "mugen_exception.h"
#include "character.h"
#include "mugen_animation.h"
#include "ast/all.h"
#include <math.h>

/* TODO:
 * 1. Change RuntimeValue into an object that stores pointers so that the
 * overhead of copying values is not so high.
 * 2. 1 necessitates garbage collection. Implement precise mark/sweep.
 * 3. Convert interpreter into a compiler. Refresher
 *   (define intepreter (lambda (env) (lambda (input) (eval env input))))
 *   (define compiler (lambda (input) (let ([compiled (compile input)])
 *                       (lambda (env) (eval env compiled)))))
 * 4. Implement simple optimizations: constant folding, dead code elimintation.
 */

using namespace std;

namespace Mugen{

string toString(const RuntimeValue & value){
    if (value.isString()){
        return value.getStringValue();
    }
    throw MugenException("Not a string");
}

double toNumber(const RuntimeValue & value){
    if (value.isDouble()){
        return value.getDoubleValue();
    }
    throw MugenException("Not a number");
}

bool toBool(const RuntimeValue & value){
    if (value.isBool()){
        return value.getBoolValue();
    }
    throw MugenException("Not a bool");
}

/* a meta-circular evaluator! */
class Evaluator: public Ast::Walker {
public:
    Evaluator(const Environment & environment):
        environment(environment){
        }

    const Environment & environment;
    RuntimeValue result;

    /* value1 == value2 */
    RuntimeValue same(const RuntimeValue & value1, const RuntimeValue & value2){
        switch (value1.type){
            case RuntimeValue::ListOfString : {
                switch (value2.type){
                    case RuntimeValue::String : {
                        const vector<string> & strings = value1.strings_value;
                        for (vector<string>::const_iterator it = strings.begin(); it != strings.end(); it++){
                            const string & check = *it;
                            if (check == value2.string_value){
                                return RuntimeValue(true);
                            }
                        }
                        return RuntimeValue(false);
                    }
                }
                break;
            }
            case RuntimeValue::String : {
                switch (value2.type){
                    case RuntimeValue::ListOfString : {
                        return same(value2, value1);
                    }
                    case RuntimeValue::String : {
                        return toString(value1) == toString(value2);
                    }
                }
                break;
            }
            case RuntimeValue::Double : {
                switch (value2.type){
                    case RuntimeValue::Double : {
                        double epsilon = 0.0000001;
                        return RuntimeValue(fabs(value1.getDoubleValue() - value2.getDoubleValue()) < epsilon);
                    }
                }
                break;
            }
        }

        return RuntimeValue(false);
    }
   
    RuntimeValue evaluate(const Ast::Value * value){
        return Mugen::evaluate(value, environment);
    }

    RuntimeValue evalIdentifier(const Ast::Identifier & identifier){
        if (identifier == "command"){
            return RuntimeValue(environment.getCommands());
        }

        if (identifier == "anim"){
            return RuntimeValue(environment.getCharacter().getAnimation());
        }

        if (identifier == "alive"){
            /* FIXME */
            return RuntimeValue(true);
        }

        if (identifier == "p2statetype"){
            /* FIXME */
            return RuntimeValue(0);
        }

        if (identifier == "animtime"){
            return RuntimeValue(environment.getCharacter().getCurrentAnimation()->animationTime());
        }

        if (identifier == "animelem"){
            /* FIXME! */
            return RuntimeValue(0);
        }

        if (identifier == "time"){
            /* FIXME! */
            return RuntimeValue(0);
        }

        if (identifier == "A"){
            /* FIXME */
            return RuntimeValue(string("A"));
        }
        
        if (identifier == "S"){
            /* FIXME */
            return RuntimeValue(string("S"));
        }

        if (identifier == "C"){
            /* FIXME */
            return RuntimeValue(string("C"));
        }

        if (identifier == "statetype"){
            /* FIXME */
            return RuntimeValue(string("S"));
        }

        /* true if the player has control */
        if (identifier == "ctrl"){
            /* FIXME */
            return RuntimeValue(true);
        }

        if (identifier == "stateno"){
            return RuntimeValue(environment.getCharacter().getCurrentState());
        }

        if (identifier == "power"){
            return RuntimeValue(environment.getCharacter().getPower());
        }

        if (identifier == "velocity.walk.back.x"){
            return RuntimeValue(environment.getCharacter().getWalkBackX());
        }

        if (identifier == "velocity.walk.fwd.x"){
            return RuntimeValue(environment.getCharacter().getWalkForwardX());
        }

        if (identifier == "velocity.run.fwd.x"){
            return RuntimeValue(environment.getCharacter().getRunForwardX());
        }
        
        if (identifier == "velocity.run.back.x"){
            return RuntimeValue(environment.getCharacter().getRunBackX());
        }
        
        if (identifier == "velocity.run.back.y"){
            return RuntimeValue(environment.getCharacter().getRunBackY());
        }

        ostringstream out;
        out << "Unknown identifier '" << identifier.toString() << "'";
        throw MugenException(out.str());
    }

    virtual void onIdenfitier(const Ast::Identifier & identifier){
        result = evalIdentifier(identifier);
    }

    RuntimeValue evalKeyword(const Ast::Keyword & keyword){
        if (keyword == "vel x"){
            return RuntimeValue(environment.getCharacter().getXVelocity());
        }

        if (keyword == "vel y"){
            return RuntimeValue(environment.getCharacter().getYVelocity());
        }
        
        if (keyword == "pos y"){
            return RuntimeValue(environment.getCharacter().getYPosition());
        }
        
        if (keyword == "p2bodydist x"){
            /* FIXME */
            return RuntimeValue(0);
        }

        ostringstream out;
        out << "Unknown keyword '" << keyword.toString() << "'";
        throw MugenException(out.str());
    }

    virtual void onKeyword(const Ast::Keyword & keyword){
        result = evalKeyword(keyword);
    }

    RuntimeValue evalString(const Ast::String & string_value){
        string out;
        string_value >> out;
        return RuntimeValue(out);
    }

    virtual void onString(const Ast::String & string){
        result = evalString(string);
    }

    RuntimeValue evalFunction(const Ast::Function & function){
        if (function == "const"){
            return evaluate(function.getArg1());
        }

        if (function == "abs"){
            return RuntimeValue(fabs(toNumber(evaluate(function.getArg1()))));
        }

        /* Gets the animation-time elapsed since the start of a specified element
         * of the current animation action. Useful for synchronizing events to
         * elements of an animation action.
         *
         * (reminder: first element of an action is element 1, not 0)
         */
        if (function == "animelemtime"){
            /* FIXME */
            return RuntimeValue(0);
        }

        ostringstream out;
        out << "Unknown function '" << function.toString() << "'";
        throw MugenException(out.str());
    }

    virtual void onFunction(const Ast::Function & string){
        result = evalFunction(string);
    }

    RuntimeValue evalNumber(const Ast::Number & number){
        double x;
        number >> x;
        return RuntimeValue(x);
    }

    virtual void onNumber(const Ast::Number & number){
        result = evalNumber(number);
    }

    virtual RuntimeValue evalExpressionInfix(const Ast::ExpressionInfix & expression){
        Global::debug(1) << "Evaluate expression " << expression.toString() << endl;
        using namespace Ast;
        switch (expression.getExpressionType()){
            case ExpressionInfix::Or : {
                return RuntimeValue(toBool(evaluate(expression.getLeft())) ||
                                    toBool(evaluate(expression.getRight())));
            }
            case ExpressionInfix::XOr : {
                return RuntimeValue(toBool(evaluate(expression.getLeft())) ^
                                    toBool(evaluate(expression.getRight())));
            }
            case ExpressionInfix::And : {
                return RuntimeValue(toBool(evaluate(expression.getLeft())) &&
                                    toBool(evaluate(expression.getRight())));
            }
            case ExpressionInfix::BitwiseOr : {
                return RuntimeValue((int)toNumber(evaluate(expression.getLeft())) |
                                    (int)toNumber(evaluate(expression.getRight())));
            }
            case ExpressionInfix::BitwiseXOr : {
                return RuntimeValue((int)toNumber(evaluate(expression.getLeft())) ^
                                    (int)toNumber(evaluate(expression.getRight())));
            }
            case ExpressionInfix::BitwiseAnd : {
                return RuntimeValue((int)toNumber(evaluate(expression.getLeft())) &
                                    (int)toNumber(evaluate(expression.getRight())));
            }
            case ExpressionInfix::Assignment : {
                break;
            }
            case ExpressionInfix::Equals : {
                return same(evaluate(expression.getLeft()), evaluate(expression.getRight()));
                break;
            }
            case ExpressionInfix::Unequals : {
                return RuntimeValue(!toBool(same(evaluate(expression.getLeft()), evaluate(expression.getRight()))));
            }
            case ExpressionInfix::GreaterThanEquals : {
                return RuntimeValue(toNumber(evaluate(expression.getLeft())) >= toNumber(evaluate(expression.getRight())));
            }
            case ExpressionInfix::GreaterThan : {
                return RuntimeValue(toNumber(evaluate(expression.getLeft())) > toNumber(evaluate(expression.getRight())));
            }
            case ExpressionInfix::LessThanEquals : {
                return RuntimeValue(toNumber(evaluate(expression.getLeft())) <= toNumber(evaluate(expression.getRight())));
            }
            case ExpressionInfix::LessThan : {
                return RuntimeValue(toNumber(evaluate(expression.getLeft())) < toNumber(evaluate(expression.getRight())));
            }
            case ExpressionInfix::Add : {
                return RuntimeValue(toNumber(evaluate(expression.getLeft())) + toNumber(evaluate(expression.getRight())));
            }
            case ExpressionInfix::Subtract : {
                return RuntimeValue(toNumber(evaluate(expression.getLeft())) - toNumber(evaluate(expression.getRight())));
            }
            case ExpressionInfix::Multiply : {
                return RuntimeValue(toNumber(evaluate(expression.getLeft())) * toNumber(evaluate(expression.getRight())));
            }
            case ExpressionInfix::Divide : {
                /* FIXME: catch divide by 0 */
                return RuntimeValue(toNumber(evaluate(expression.getLeft())) / toNumber(evaluate(expression.getRight())));
            }
            case ExpressionInfix::Modulo : {
                return RuntimeValue((int)toNumber(evaluate(expression.getLeft())) % (int)toNumber(evaluate(expression.getRight())));
            }
            case ExpressionInfix::Power : {
                return RuntimeValue(pow(toNumber(evaluate(expression.getLeft())), toNumber(evaluate(expression.getRight()))));
            }
        }

        throw MugenException("Unknown expression");
    }

    virtual void onExpressionInfix(const Ast::ExpressionInfix & expression){
        result = evalExpressionInfix(expression);
    }

    RuntimeValue evalExpressionUnary(const Ast::ExpressionUnary & expression){
        switch (expression.getExpressionType()){
            case Ast::ExpressionUnary::Not : {
                return RuntimeValue(!toBool(evaluate(expression.getExpression())));
            }
            case Ast::ExpressionUnary::Minus : {
                return RuntimeValue(-toNumber(evaluate(expression.getExpression())));
            }
            case Ast::ExpressionUnary::Negation : {
                return RuntimeValue(~(int)toNumber(evaluate(expression.getExpression())));
            }
        }
        throw MugenException("Unknown expression");
    }
    
    virtual void onExpressionUnary(const Ast::ExpressionUnary & expression){
        result = evalExpressionUnary(expression);
    }
};

RuntimeValue evaluate(const Ast::Value * value, const Environment & environment){
    try{
        Evaluator eval(environment);
        value->walk(eval);
        return eval.result;
    } catch (const MugenException & e){
        ostringstream out;
        out << "Error while evaluating expression `" << value->toString() << "': " << e.getReason();
        throw MugenException(out.str());
    }
}

}
