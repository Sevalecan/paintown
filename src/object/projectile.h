#ifndef _paintown_projectile_h
#define _paintown_projectile_h

#include "object_attack.h"
#include "util/load_exception.h"

class Token;
class Animation;

class Projectile: public ObjectAttack {
public:
	Projectile( Token * token ) throw( LoadException );
	Projectile( const Projectile * projectile );
	
	virtual void act( vector< Object * > * others, World * world, vector< Object * > * add );
	virtual void draw( Bitmap * work, int rel_x );
	virtual void grabbed( Object * obj );
	virtual void unGrab();
	virtual Object * copy();
	virtual const std::string & getAttackName();
	virtual bool isAttacking();
	virtual bool collision( ObjectAttack * obj );
	virtual int getDamage() const;
	virtual bool isCollidable( Object * obj );
	virtual bool isGettable();
	virtual const int getWidth() const;
	virtual const int getHeight() const;
	virtual void getAttackCoords( int & x, int & y);
	virtual const double minZDistance() const;
	virtual void attacked( Object * something, vector< Object * > & objects );

	virtual ~Projectile();

protected:
	std::string name;

	Animation * main;
	Animation * death;
};

#endif
