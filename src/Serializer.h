#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cstdint>
#include <unordered_map>
#include <algorithm>

using namespace std;

template <typename MostDerived, typename C, typename M>
ptrdiff_t MemberOffset (M C::* member)
{
	MostDerived d;
	return reinterpret_cast<char*> (&(d.*member)) - reinterpret_cast<char*> (&d);
}


struct Reference {
	intptr_t mWho;
	intptr_t mWhat;
	int mOffset;

	Reference( ) : mWho( 0 ), mWhat( 0 ), mOffset( 0 ) {

	}

	Reference( intptr_t who, intptr_t what, int offset ) : mWho( who ), mWhat( what ), mOffset( offset ) {

	}

	Reference( const void * who, const void * what, int offset ) : mWho( (intptr_t)who ), mWhat( (intptr_t)what ), mOffset( offset ) {

	}
};

class Serializer {
private:
	bool mWrite;
	fstream mFile;

	// Writer
	vector<intptr_t> mSerializedObjects;

	// Reader
	unordered_map<intptr_t, void*> mDynamicObjects;
	vector<Reference> mReferences;
public:
	Serializer( const string & filename, bool write ) : mWrite( write ) {
		mFile.open( filename, (mWrite ? ios::out : ios::in) | ios::binary );
	}

	bool IsSerialized( const void * ptr ) {
		if( mWrite ) {
			return find( mSerializedObjects.begin(), mSerializedObjects.end(), (intptr_t)ptr ) != mSerializedObjects.end();
		} else {
			return false;
		}
	}

	void Finish() {
		mFile.close();

		if( !mWrite ) {
			// Resolve references
			for( auto & ref : mReferences ) {
				char * who = (char*)mDynamicObjects[ref.mWho];
				char * what = (char*)mDynamicObjects[ref.mWhat];

				if( what && who ) {
					memcpy( who + ref.mOffset, &what, sizeof(intptr_t));
				}
			}
		}
	}
	
	// Handle integer
	void operator & ( int & i ) {		
		if( mWrite ) {
			mFile.write( (char*)&i, sizeof( i ));
		} else {
			mFile.read( (char*)&i, sizeof( i ));
		}
	}
		
	// Handle unsigned integer
	void operator & ( unsigned int & ui ) {
		if( mWrite ) {
			mFile.write( (char*)&ui, sizeof( ui ));
		} else {
			mFile.read( (char*)&ui, sizeof( ui ));
		}
	}
		
	// Handle unsigned short
	void operator & ( unsigned short & us ) {
		if( mWrite ) {
			mFile.write( (char*)&us, sizeof( us ));
		} else {
			mFile.read( (char*)&us, sizeof( us ));
		}
	}
	
	// Handle short
	void operator & ( short & s ) {
		if( mWrite ) {
			mFile.write( (char*)&s, sizeof( s ));
		} else {
			mFile.read( (char*)&s, sizeof( s ));
		}
	}
		
	// Handle char
	void operator & ( char & c ) {
		if( mWrite ) {
			mFile.write( (char*)&c, sizeof( c ));
		} else {
			mFile.read( (char*)&c, sizeof( c ));
		}
	}
	
	// Handle unsigned char
	void operator & ( unsigned char & uc ) {
		if( mWrite ) {
			mFile.write( (char*)&uc, sizeof( uc ));
		} else {
			mFile.read( (char*)&uc, sizeof( uc ));
		}
	}

	// Handle float
	void operator & ( float & f ) {
		if( mWrite ) {
			mFile.write( (char*)&f, sizeof( f ));
		} else {
			mFile.read( (char*)&f, sizeof( f ));
		}
	}

	// Handle string
	void operator & ( string & str ) {
		if( mWrite ) {
			for( auto symbol : str ) {
				mFile.write( &symbol, sizeof( symbol ));
			}
			char end = '\0';
			mFile.write( &end, sizeof( end ));
		} else {
			while( !mFile.eof()) {
				char symbol;
				mFile.read( &symbol, sizeof( symbol ));
				if( symbol == '\0' ) {
					break;
				} else {
					str.push_back( symbol );
				}
			}
		}
	}

	// Handle C string
	void operator & ( const char * str ) {
		if( mWrite ) {
			while( *str != '\0' ) {
				mFile.write( str, sizeof( *str ));
				++str;
			}
			char end = '\0';
			mFile.write( &end, sizeof( end ));
		} else {

		}
	}

	// Handle vector of objects
	template<class T> 
	void operator & ( std::vector<T> & v ) {
		if( mWrite ) {
			auto count = v.size();
			(*this) & count;
			for( size_t i = 0; i < v.size(); ++i ) {
				v[i].Serialize( *this );			
			}
		} else {
			int count;
			(*this) & count;
			v.resize( count );
			for( size_t i = 0; i < v.size(); ++i ) {			
				v[i].Serialize( *this );
			}
		}
	}

	// Handle vector of pointers
	template<class T> 
	void operator & ( std::vector<T*> & v ) {
		if( mWrite ) {
			auto count = v.size();
			(*this) & count;
			for( size_t i = 0; i < v.size(); ++i ) {
				(*this) & Reference( &v[i], v[i], 0 );
			}
		} else {
			int count;
			(*this) & count;
			v.resize( count );
			for( size_t i = 0; i < v.size(); ++i ) {			
				Reference ref;
				(*this) & ref;
				mDynamicObjects[ ref.mWho ] = &v[i];
			}
		}
	}
	
	// Handle reference
	void operator & ( Reference & ref ) {
		if( mWrite ) {
			mFile.write( (char*)&ref.mWho, sizeof( ref.mWho ));
			mFile.write( (char*)&ref.mWhat, sizeof( ref.mWhat ));
			mFile.write( (char*)&ref.mOffset, sizeof( ref.mOffset ));	
		} else {
			intptr_t who;
			mFile.read( (char*)&who, sizeof( who ));

			intptr_t what;
			mFile.read( (char*)&what, sizeof( what ));

			int offset;
			mFile.read( (char*)&offset, sizeof( offset ));

			ref.mWho = who;
			ref.mWhat = what;
			ref.mOffset = offset;

			mReferences.push_back( ref );
		}
	}

	// Handle pointer
	template<class T> 
	void operator & ( T * ptr ) {
		if( mWrite ) {
			intptr_t p = (intptr_t)ptr;
			mFile.write( (char*)&p, sizeof( p ));
			mSerializedObjects.push_back( p );
		} else {
			intptr_t p;
			mFile.read( (char*)&p, sizeof( p ));
			mDynamicObjects[p] = const_cast<T*>( ptr );
		}
	}
};

