#include "Serializer.h"

// Tests based on the scene graph - real and complex model

class Property {
public:
	string mName;
	int mValue;

	Property() : mValue( 0 ) {
		
	}

	Property( const string & name, int value ) : mName( name ), mValue( value ) {

	}

	virtual void Serialize( class Serializer & s ) {
		s & mName;
		s & mValue;
	}
};

class SceneNode {
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

		s & this;
		s & mName;
		s & Reference( this, mParent, MemberOffset<SceneNode>( &SceneNode::mParent ));
		s & mChildren;
		s & mProperties;
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

		s & mRadius;
	}

	virtual void PrintInfo() const {
		SceneNode::PrintInfo();

		cout << "Light data" << endl;
		cout << "Radius: " << mRadius << endl;
	}
};

void Save( const string & filename ) {
	Serializer s( filename, true );

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
	auto count = SceneNode::msNodes.size();
	s & count;
	for( int i = 0; i < count; ++i ) {
		auto node = SceneNode::msNodes[i];
		// write type
		if( dynamic_cast<Light*>( node )) {
			s & "Light";
		} else {
			s & "Node";
		}
		node->Serialize( s );
	}

	cout << endl << "Scene is deleted!" << endl;

	delete node1;
	delete node2;

	s.Finish();
}

void Load( const string & filename ) {
	Serializer s( filename, false );

	cout << endl << "Loading scene from: " << filename << endl;

	int nodeCount;
	s & nodeCount;
	for( int i = 0; i < nodeCount; ++i ) {
		SceneNode * node;
		// Read type 
		string type;
		s & type;
		// Create object
		if( type == "Light" ) {
			node = new Light;
		} else {
			node = new SceneNode;
		}		
		node->Serialize( s );
	}

	s.Finish();

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