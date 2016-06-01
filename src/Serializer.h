#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cstdint>
#include <unordered_map>

using namespace std;

class Serializer {
private:
	ofstream mFile;
public:
	Serializer( const string & filename ) {
		mFile.open( filename, ios::out | ios::binary );
	}

	void WritePointer( void * ptr ) {
		intptr_t p = (intptr_t)ptr;
		mFile.write( (char*)&p, sizeof( p ));
	}

	void WriteReference( const void * who, const void * what, int offset ) {
		intptr_t p;

		p = (intptr_t)who;
		mFile.write( (char*)&p, sizeof( p ));

		p = (intptr_t)what;
		mFile.write( (char*)&p, sizeof( p ));

		mFile.write( (char*)&offset, sizeof( offset ));
	}

	void WriteString( const string & str ) {
		for( auto symbol : str ) {
			mFile.write( &symbol, sizeof( symbol ));
		}
		char end = '\0';
		mFile.write( &end, sizeof( end ));
	}

	void WriteInteger( int i ) {
		mFile.write( (char*)&i, sizeof( i ));
	}

	void WriteFloat( float f ) {
		mFile.write( (char*)&f, sizeof( f ));
	}

	template<class T>
	void WriteStdVector( const std::vector<T> & v ) {
		WriteInteger( v.size() );
		for( size_t i = 0; i < v.size(); ++i ) {
			WriteReference( &v[i], v[i], 0 );
		}
	}

	void Finish() {
		mFile.close();
	}
};

struct Reference {
	intptr_t mWho;
	intptr_t mWhat;
	int mOffset;

	Reference( ) : mWho( 0 ), mWhat( 0 ), mOffset( 0 ) {

	}

	Reference( intptr_t who, intptr_t what, int offset ) : mWho( who ), mWhat( what ), mOffset( offset ) {

	}
};

template <typename MostDerived, typename C, typename M>
ptrdiff_t MemberOffset (M C::* member)
{
	MostDerived d;
	return reinterpret_cast<char*> (&(d.*member)) - reinterpret_cast<char*> (&d);
}

class Deserializer {
private:
	ifstream mFile;
	unordered_map<intptr_t, void*> mDynamicObjects;
	vector<Reference> mReferences;
public:
	Deserializer( const string & filename ) {
		mFile.open( filename, ios::in | ios::binary );
	}

	void ReadPointer( void * realObject ) {
		intptr_t p;
		mFile.read( (char*)&p, sizeof( p ));
		mDynamicObjects[p] = realObject;
	}

	Reference ReadReference() {
		intptr_t who;
		mFile.read( (char*)&who, sizeof( who ));

		intptr_t what;
		mFile.read( (char*)&what, sizeof( what ));

		int offset;
		mFile.read( (char*)&offset, sizeof( offset ));

		mReferences.push_back( Reference( who, what, offset ));

		return Reference( who, what, offset );
	}

	int ReadInteger( ) {
		int i;
		mFile.read( (char*)&i, sizeof( i ));
		return i;
	}

	float ReadFloat( ) {
		float f;
		mFile.read( (char*)&f, sizeof( f ));
		return f;
	}

	template<class T>
	void ReadStdVector( std::vector<T> & v ) {
		v.resize( ReadInteger());
		for( size_t i = 0; i < v.size(); ++i ) {			
			Reference ref = ReadReference();
			mDynamicObjects[ ref.mWho ] = &v[i];
		}
	}

	void Finish() {		
		mFile.close();

		ResolveReferences();
	}

	void ResolveReferences() {
		for( auto & ref : mReferences ) {
			char * who = (char*)mDynamicObjects[ref.mWho];
			char * what = (char*)mDynamicObjects[ref.mWhat];

			if( what && who ) {
				memcpy( who + ref.mOffset, &what, sizeof(intptr_t));
			}
		}
	}

	string ReadString() {
		string str;		
		while( !mFile.eof()) {
			char symbol;
			mFile.read( &symbol, sizeof( symbol ));
			if( symbol == '\0' ) {
				break;
			} else {
				str.push_back( symbol );
			}
		}
		return str;
	}
};

class Serializable {
public:
	virtual void Serialize( Serializer & s ) = 0;
	virtual void Deserialize( Deserializer & d ) = 0;
};