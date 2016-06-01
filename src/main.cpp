#include "Serializer.h"

class SceneNode : public Serializable {
public:
	SceneNode * mParent;
	string mName;
	vector<SceneNode*> mChildren;
public:
	SceneNode() : mParent( nullptr ) {

	}

	~SceneNode() {

	}

	void SetName( const string & name ) {
		mName = name;
	}

	void AttachTo( SceneNode * parent ) {
		mParent = parent;
		mParent->mChildren.push_back( this );
	}

	virtual void Serialize( Serializer & s ) {
		s.WritePointer( this );
		s.WriteString( mName );
		s.WriteReference( this, mParent, MemberOffset<SceneNode>( &SceneNode::mParent ));
		s.WriteStdVector( mChildren );
	}

	virtual void Deserialize( Deserializer & d ) {
		d.ReadPointer( this );
		mName = d.ReadString();
		d.ReadReference();
		d.ReadStdVector( mChildren );
	}

	static SceneNode * Create( Deserializer & d ) {
		SceneNode * node = new SceneNode();
		node->Deserialize( d );
		return node;
	}
};

class Light : public SceneNode {
private:
	float mRadius;
public:
	Light() : mRadius( 1.0f ) {

	}

	void SetRadius( float r ) {
		mRadius = r;
	}

	virtual void Serialize( Serializer & s ) {
		SceneNode::Serialize( s );

		s.WriteFloat( mRadius );
	}

	virtual void Deserialize( Deserializer & d ) {
		SceneNode::Deserialize( d );
		mRadius = d.ReadFloat();
	}

	static Light * Create( Deserializer & d ) {
		Light * node = new Light();
		node->Deserialize( d );
		return node;
	}
};

void Save( const string & filename ) {
	Serializer s( filename );

	SceneNode * node1 = new SceneNode();
	node1->SetName( "Node 1" );

	Light * node2 = new Light();
	node2->SetName( "Node 2" );
	node2->SetRadius( 35.0f );

	node1->AttachTo( node2 );

	node1->Serialize( s );
	node2->Serialize( s );

	delete node1;
	delete node2;

	s.Finish();
}

void Load( const string & filename ) {
	Deserializer d( filename );

	SceneNode * node1 = SceneNode::Create( d );
	SceneNode * node2 = Light::Create( d );

	d.Finish();

	delete node1;
	delete node2;	
}

int main() {
	string filename = "dump.bin";

	///////////////
	// Serialize
	Save( filename );
	
	///////////////
	// Deserialize
	Load( filename );

	return 0;
}