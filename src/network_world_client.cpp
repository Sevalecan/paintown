#include "network_world_client.h"
#include "network.h"
#include "level/scene.h"
#include "globals.h"
#include "level/blockobject.h"
#include "util/funcs.h"
#include "factory/object_factory.h"
#include <pthread.h>
#include <string.h>

#include "object/character.h"
#include "object/cat.h"

static void * handleMessages( void * arg ){
	NetworkWorldClient * world = (NetworkWorldClient *) arg;
	NLsocket socket = world->getServer();
	// pthread_mutex_t * lock = world->getLock();
	
	try{
		while ( 1 ){
			Network::Message m( socket );
			// pthread_mutex_lock( lock );
			world->addIncomingMessage( m );
			Global::debug( 2 ) << "Received path '" << m.path << "'" << endl;
			// pthread_mutex_unlock( lock );
		}
	} catch ( const Network::NetworkException & n ){
		Global::debug( 0 ) << "Network exception" << endl;
	}

	return NULL;
}
	
NetworkWorldClient::NetworkWorldClient( NLsocket server, const std::vector< Object * > & players, const string & path, int screen_size ) throw ( LoadException ):
World( players, path, screen_size ),
server( server ){
	pthread_mutex_init( &message_mutex, NULL );
	pthread_create( &message_thread, NULL, handleMessages, this );
}
	
void NetworkWorldClient::addIncomingMessage( const Network::Message & message ){
	pthread_mutex_lock( &message_mutex );
	incoming.push_back( message );
	pthread_mutex_unlock( &message_mutex );
}
	
vector< Network::Message > NetworkWorldClient::getIncomingMessages(){
	vector< Network::Message > m;
	pthread_mutex_lock( &message_mutex );
	m = incoming;
	incoming.clear();
	pthread_mutex_unlock( &message_mutex );
	return m;
}

bool NetworkWorldClient::uniqueObject( Object * object ){
	for ( vector< Object * >::iterator it = objects.begin(); it != objects.end(); it++ ){
		Object * o = *it;
		if ( o->getId() == object->getId() ){
			return false;
		}
	}
	return true;
}
	
void NetworkWorldClient::handleMessage( Network::Message & message ){
	if ( message.id == 0 ){
		int type;
		message >> type;
		switch ( type ){
			case CREATE_CHARACTER : {
				int alliance;
				int id;
				int map;
				string path = Util::getDataPath() + "/" + message.path;
				message >> alliance >> id >> map;
				BlockObject block;
				block.setType( ObjectFactory::OBJECT_NETWORK_CHARACTER );
				block.setMap( map );
				block.setPath( path );
				Character * character = (Character *) ObjectFactory::createObject( &block );
				if ( character == NULL ){
					Global::debug( 0 ) << "Could not create character!" << endl;
					break;
				}
				Global::debug( 1 ) << "Create '" << path << "' with id " << id << " alliance " << alliance << endl;
				character->setId( id );
				character->setAlliance( alliance );
				character->setX( 200 );
				character->setY( 0 );
				character->setZ( 150 );
				if ( uniqueObject( character ) ){
					addObject( character );
				} else {
					delete character;
				}
				break;
			}
			case CREATE_CAT : {
				int id;
				message >> id;
				string path = Util::getDataPath() + "/" + message.path;
				BlockObject block;
				block.setType( ObjectFactory::OBJECT_CAT );
				block.setPath( path );
				block.setCoords( 200, 150 );
				Cat * cat = (Cat *) ObjectFactory::createObject( &block );
				if ( cat == NULL ){
					Global::debug( 0 ) << "Could not create cat" << endl;
					break;
				}

				cat->setY( 0 );
				if ( uniqueObject( cat ) ){
					addObject( cat );
				} else {
					delete cat;
				}

				break;
			}
			case CREATE_BANG : {
				int x, y, z;
				message >> x >> y >> z;
				Object * addx = bang->copy();
				addx->setX( x );
				addx->setY( 0 );
				addx->setZ( y+addx->getHeight()/2 );
				addx->setHealth( 1 );
				addx->setId( (unsigned int) -1 );
				addObject( addx );
				break;
			}
			case NEXT_BLOCK : {
				int block;
				message >> block;
				scene->advanceBlocks( block );
				break;
			}
		}
	} else {
		for ( vector< Object * >::iterator it = objects.begin(); it != objects.end(); it++ ){
			Object * o = *it;
			if ( o->getId() == message.id ){
				o->interpretMessage( message );
			}
		}
	}
}

void NetworkWorldClient::addMessage( Network::Message m ){
	outgoing.push_back( m );
}
	
void NetworkWorldClient::doScene( int min_x, int max_x ){
	vector< Object * > objs;
	scene->act( min_x, max_x, &objs );
	for ( vector< Object * >::iterator it = objs.begin(); it != objs.end(); it++ ){
		delete *it;
	}
}

void NetworkWorldClient::sendMessage( const Network::Message & message, NLsocket socket ){
	Global::debug( 2 ) << "Sending message to client" << endl;
	Network::send16( socket, message.id );
	Global::debug( 2 ) << "Sent message id " << message.id << endl;
	Network::sendBytes( socket, message.data, Network::DATA_SIZE );
	if ( message.path != "" ){
		Global::debug( 2 ) << "Send message length " << message.path.length() << endl;
		Network::send16( socket, message.path.length() + 1 );
		Global::debug( 2 ) << "Send path '" << message.path << "'" << endl;
		Network::sendStr( socket, message.path );
	} else {
		Network::send16( socket, -1 );
	}
}

void NetworkWorldClient::act(){
	
	if ( quake_time > 0 ){
		quake_time--;
	}

	vector< Object * > added_effects;
	for ( vector< Object * >::iterator it = objects.begin(); it != objects.end(); it++ ){
		Object * o = *it;
		o->act( &objects, this, &added_effects );
		if ( o->getZ() < getMinimumZ() ){
			o->setZ( getMinimumZ() );
		}
		if ( o->getZ() > getMaximumZ() ){
			o->setZ( getMaximumZ() );
		}
	}
	objects.insert( objects.end(), added_effects.begin(), added_effects.end() );

	double lowest = 9999999;
	for ( vector< PlayerTracker >::iterator it = players.begin(); it != players.end(); it++ ){
		Object * player = it->player;
		double mx = player->getX() - screen_size / 2;
		if ( it->min_x < mx ){
			it->min_x++;
		}
	
		if ( it->min_x + screen_size >= scene->getLimit() ){
			it->min_x = scene->getLimit() - screen_size;
		}

		if ( it->min_x < lowest ){
			lowest = it->min_x;
		}
		
		if ( player->getX() < it->min_x ){
			player->setX( it->min_x );
		}

		if ( player->getX() > scene->getLimit() ){
			player->setX( scene->getLimit() );
		}
		if ( player->getZ() < getMinimumZ() ){
			player->setZ( getMinimumZ() );
		}
		if ( player->getZ() > getMaximumZ() ){
			player->setZ( getMaximumZ() );
		}
	}

	doScene( 0, 0 );

	vector< Network::Message > messages = getIncomingMessages();
	for ( vector< Network::Message >::iterator it = messages.begin(); it != messages.end(); it++ ){
		handleMessage( *it );
	}

	for ( vector< Object * >::iterator it = objects.begin(); it != objects.end(); ){
		if ( (*it)->getHealth() <= 0 ){
			(*it)->died( added_effects );
			if ( ! isPlayer( *it ) ){
				delete *it;
			}
			it = objects.erase( it );
		} else ++it;
	}

	for ( vector< Network::Message >::iterator it = outgoing.begin(); it != outgoing.end(); it++ ){
		Network::Message & m = *it;
		sendMessage( m, getServer() );
	}
	outgoing.clear();
}
