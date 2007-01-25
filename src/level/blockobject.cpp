#include <iostream>

#include "blockobject.h"
#include "util/load_exception.h"
#include "util/token_exception.h"
#include "util/token.h"

BlockObject::BlockObject( Token * tok ) throw ( LoadException ):
type( -1 ),
map( 0 ),
health( 1 ),
coords_x( 0 ),
coords_z( 0 ){
	while ( tok->hasTokens() ){
		try{
			Token * current;
			*tok >> current;

			if ( *current == "type" ){
				string k;
				*current >> k;
				if ( k == "1up" ){
					type = OBJECT_1UP;
				} else if ( k == "enemy" ){
					type = OBJECT_ENEMY;
				} else {
					tok->print(" ");
					throw LoadException("Not a valid type");
				}
			} else if ( *current == "path" ){
				string n;
				*current >> n;
				setPath( n );
			} else if ( *current == "name" ){
				string n;
				*current >> n;
				if ( n.length() > 0 && (n[0] >= 'a' && n[0] <= 'z') )
					n[0] = n[0] - 'a' + 'A';
				setName( n );
			} else if ( *current == "alias" ){
				string n;
				*current >> n;
				if ( n.length() > 0 && (n[0] >= 'a' && n[0] <= 'z') )
					n[0] = n[0] - 'a' + 'A';
				setAlias( n );
			} else if ( *current == "coords" ){
				int x, z;
				*current >> x >> z;
				setCoords( x, z );
			} else if ( *current == "health" ){
				int h;
				*current >> h;
				setHealth( h );
			} else if ( *current == "at" ){
				/* not quite sure what 'at' is */
			} else if ( *current == "map" ){
				int m;
				*current >> m;
				setMap( m );
			} else {
				cout<<"Unhandled blockobject token"<<endl;
				current->print(" ");
			}

		} catch( const TokenException & te ){
			throw LoadException("Blockobject parse exception");
		}
	}

	/* give it some stupid name */
	if ( getName() == "" ){
		name = "a";
		name += getType();
	}
	if ( getAlias() == "" )
		setAlias( getName() );

	if ( getPath() == "" ){
		cout<<endl;
		cout<<"**WARNING**"<<endl;
		tok->print(" ");
		string str("No path given for ");
		str += getName();
		throw LoadException( str );
	}
}

BlockObject::~BlockObject(){
}
