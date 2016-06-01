#include "Serializer.h"

// Tests based on the scene graph - real and complex model

class Property : public Serializable {
public:
	string mName;
	int mValue;

	Property() : mValue( 0 ) {
		
	}

	Property( const string & name, int value ) : mName( name ), mValue( value ) {

	}

	virtual void Serialize( class Serializer & s ) {
		s.WriteString( mName );
		s.WriteInteger( mValue );
	}

	virtual void Deserialize( class Deserializer & d ) {
		mName = d.ReadString();
		mValue = d.ReadInteger( );
	}
};

class SceneNode : public Serializable {
public:
	string mName;
	SceneNode * mParent;	
	vector<SceneNode*> mChildren;
	vector<Property> mProperties;
	static vector<SceneNode*> msNodes;
public:
	SceneNode() : mParent( nullptr ) {
		msNodes.push_back( this );
	}

	~SceneNode() {
		msNodes.erase( find( msNodes.begin(), msNodes.end(), this ));
	}

	void AddProperty( const Property & p ) {
		mProperties.push_back( p );
	}

	void SetName( const string & name ) {
		mName = name;
	}

	string GetName() const {
		return mName;
	}

	const vector<Property> & GetProperties() const {
		return mProperties;
	}

	SceneNode * GetParent() const {
		return mParent;
	}

	void AttachTo( SceneNode * parent ) {
		mParent = parent;
		mParent->mChildren.push_back( this );
	}

	virtual void PrintInfo() const {
		cout << endl;
		cout << "Name: " << mName << endl;
		cout << "Parent: " << mParent << ": " << (mParent ? mParent->mName : " " ) << endl;		
		for( auto child : mChildren ) {
			cout << "Child: " << child << ": " << child->mName << endl;
		}
		for( auto & p : mProperties ) {
			cout << "Property: " << p.mName << ": " << p.mValue << endl;
		}		
	}

	virtual void Serialize( Serializer & s ) {
		if( s.IsSerialized( this )) return;

		s.WritePointer( this );
		s.WriteString( mName );
		s.WriteReference( this, mParent, MemberOffset<SceneNode>( &SceneNode::mParent ));
		s.WriteStdVectorOfPointers( mChildren );
		s.WriteStdVectorOfObjects( mProperties );
	}

	virtual void Deserialize( Deserializer & d ) {
		d.ReadPointer( this );
		mName = d.ReadString();
		d.ReadReference();
		d.ReadStdVectorOfPointers( mChildren );
		d.ReadStdVectorOfObjects( mProperties );
	}

	static SceneNode * Create( Deserializer & d ) {
		SceneNode * node = new SceneNode();
		node->Deserialize( d );
		return node;
	}
};

vector<SceneNode*> SceneNode::msNodes;

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
		if( s.IsSerialized( this )) return;

		SceneNode::Serialize( s );

		s.WriteFloat( mRadius );
	}

	virtual void PrintInfo() const {
		SceneNode::PrintInfo();

		cout << "Light data" << endl;
		cout << "Radius: " << mRadius << endl;
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

	cout << "Creating test scene..." << endl;
	// Create test scene
	SceneNode * node1 = new SceneNode();
	node1->SetName( "Node 1" );
	node1->AddProperty( Property( "Some property", 42 ));

	Light * node2 = new Light();
	node2->SetName( "Node 2" );
	node2->SetRadius( 35.0f );
	node2->AddProperty( Property( "Fancy property", 666 ));
	
	node1->AttachTo( node2 );

	
	for( auto node : SceneNode::msNodes ) {
		node->PrintInfo();
	}

	cout << "Test scene created!" << endl;

	cout << "Saving scene to: " << filename << endl;

	// Serialize all nodes
	s.WriteInteger( SceneNode::msNodes.size() );
	for( int i = 0; i < SceneNode::msNodes.size(); ++i ) {
		auto node = SceneNode::msNodes[i];
		// write type
		if( dynamic_cast<Light*>( node )) {
			s.WriteString( "Light" );
		} else {
			s.WriteString( "Node" );
		}
		node->Serialize( s );
	}

	cout << endl << "Scene is deleted!" << endl;

	delete node1;
	delete node2;

	s.Finish();
}

void Load( const string & filename ) {
	Deserializer d( filename );

	cout << endl << "Loading scene from: " << filename << endl;

	int nodeCount = d.ReadInteger();
	for( int i = 0; i < nodeCount; ++i ) {
		SceneNode * node;
		// Read type 
		string type = d.ReadString();
		// Create object
		if( type == "Light" ) {
			node = Light::Create( d );
		} else {
			node = SceneNode::Create( d );
		}		
	}

	d.Finish();

	for( auto node : SceneNode::msNodes ) {
		node->PrintInfo();
	}
}

int main() {
	string filename = "dump.bin";

	///////////////
	// Serialize
	Save( filename );
	
	///////////////
	// Deserialize
	Load( filename );

	system( "pause" );

	return 0;
}