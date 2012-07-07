#include "export.h"
#include "scene/hierfile.h"

namespace Export {

bool SaveHierachy( MeshMod::ScenePtr scene, 
                              	Core::ResourceManifestEntryVector& manifest, 
                               	const Core::FilePath pOutFilename ) {
	using namespace Scene;
	std::ostringstream outStream;
	std::ostringstream nodeStream;
	std::ostringstream linkStream;
	std::map<std::string,std::string> stringTable;

	// produce a wob node header 
	outStream << "// hierachy file\n";
	outStream << ".type u32\n";
	outStream << HierType << "\t// HIER\n";	// magic

	// collect nodes
	std::vector< MeshMod::SceneNodePtr > nodeArray;
	std::stack< MeshMod::SceneNodePtr > nodes;
	int nodeCounter = 0;

	for( auto nodeIt = scene->sceneNodes.begin(); nodeIt != scene->sceneNodes.end(); ++nodeIt ) {
		nodes.push( *nodeIt );
	}

	while( !nodes.empty() ) {
		MeshMod::SceneNodePtr n = nodes.top();
		nodes.pop(); ++nodeCounter;
		nodeArray.push_back( n );
		for( uint32_t i=0;i < n->getChildCount();++i ) {
			nodes.push( n->getChild(i) );
		}
	}

	assert( nodeArray.size() == nodeCounter );
	outStream << "(u16) " << nodeCounter << "\t\t\t\t\t // num nodes\n";
	outStream << "(u8) " << (int)HierVersion << "\t\t\t\t\t // version\n";
	if( scene->properties.size() > 0 ) {
		outStream << "(u8) " << HFF_ENVIRONMENT << "\t\t\t\t\t // flags = HFF_ENVIRONMENT\n";
	} else {
		outStream << "(u8) " << 0 << "\t\t\t\t\t // flags\n";
	}
	outStream << "endLab - beginLab \t\t\t\t\t // total size\n";

	for( std::vector< MeshMod::SceneNodePtr >::const_iterator it = nodeArray.begin();
		it != nodeArray.end();
		++it ) {
			nodeStream << "//---------------------------------------------\n";
			nodeStream << ".align 8\n";
			linkStream << "//------------------------------------------\n";
			MeshMod::SceneNodePtr node = *it;
			int nodeIndex = (int) std::distance<std::vector< MeshMod::SceneNodePtr >::const_iterator>( nodeArray.begin(), it );
			nodeStream << "Node_"  << nodeIndex << ": \t\t\t\t\t // node Start\n";

			if( node->type == "Joint" ) {
				nodeStream << "(u16)" << HNT_JOINT << "\t\t\t\t\t// type = JOINT\n";
				if( (*it)->properties.size() > 0 ) {
					nodeStream << "(u16) " << HNF_PROPERTIES << "\t\t\t\t\t // flags = HNF_PROPERTIES\n";
				} else {
					nodeStream << "(u16) " << 0 << "\t\t\t\t\t // flags\n";
				}
				nodeStream << ".align 8\n";
				nodeStream << "0,0" << "\t\t\t\t\t// Node parameters ptr\n";
			} else { // also covers if( node->m_type == "Mesh" ) {

				MeshMod::MeshPtr mesh;
				// TODO what to do with multiple nodes?
				for( unsigned int objNum = 0; objNum < node->getObjectCount();++objNum ) {
					MeshMod::SceneObjectPtr obj = node->getObject(objNum);
					if( obj->getType() == "Mesh" ) {
						mesh = std::dynamic_pointer_cast<MeshMod::Mesh>( obj );
						break;
					}
				}
				if(mesh) {
					nodeStream << "(u16)" << HNT_MESH << "\t\t\t\t\t// type = MESH\n";
					if( (*it)->properties.size() > 0 ) {
						nodeStream << "(u16) " << HNF_PROPERTIES << "\t\t\t\t\t // flags = HNF_PROPERTIES\n";
					} else {
						nodeStream << "(u16) " << 0 << "\t\t\t\t\t // flags\n";
					}
					nodeStream << ".align 8\n";
	
					std::ostringstream meshLabName;
					meshLabName <<  "MeshName_N" << nodeIndex << "Str";
					nodeStream << "0, " << meshLabName.str() << " - beginLab" << "\t\t\t\t\t// node parameters\n";
					auto baseName = pOutFilename.RemoveExtension().BaseName().value();
					auto meshName = Core::FilePath( baseName + "_" + mesh->getName()).RemoveExtension().value();

					stringTable[ meshLabName.str() + ":" ] = meshName;

				} else {
					// default to node as we have no mesh?
					nodeStream << "(u16)" << HNT_NODE << "\t\t\t\t\t// type = NODE\n";
					if( (*it)->properties.size() > 0 ) {
						nodeStream << "(u16) " << HNF_PROPERTIES << "\t\t\t\t\t // flags = HNF_PROPERTIES\n";
					} else {
						nodeStream << "(u16) " << 0 << "\t\t\t\t\t // flags\n";
					}
					nodeStream << ".align 8\n";
					nodeStream << "0,0" << "\t\t\t\t\t// node parameters \n";
				}

			}

			nodeStream << ".type float\n";
			nodeStream	<<  (float) node->transform.position[0] << ", "
						<<  (float) node->transform.position[1] << ", "
						<<  (float) node->transform.position[2] << "\t\t\t\t\t// Node Position\n";
			nodeStream	<<  (float) node->transform.orientation[0] << ", "
						<<  (float) node->transform.orientation[1] << ", "
						<<  (float) node->transform.orientation[2] << ", "
						<<  (float) node->transform.orientation[3] << "\t\t\t\t\t// Node Rotation (quat)\n";
			nodeStream	<<  (float) node->transform.scale[0] << ", "
						<<  (float) node->transform.scale[1] << ", "  
						<<  (float) node->transform.scale[2] << "\t\t\t\t\t// Node Scale\n";
			nodeStream << ".type u32\n";
			nodeStream << ".align 8\n";

			nodeStream << "0, NodeTree" << nodeIndex << " - beginLab"<< "\t\t\t\t\t // pointer to children node tree\n";
			linkStream << "NodeTree" << nodeIndex << ":\n";
			linkStream << (uint32_t)node->getChildCount() << "\t\t\t\t\t // Number of Childen\n";
			for( unsigned int i=0;i < node->getChildCount();++i ) {
				linkStream << std::distance( nodeArray.begin(), std::find( nodeArray.begin(), nodeArray.end(), node->getChild(i) ) );
				if( i != node->getChildCount()-1 ) {
					linkStream << " , ";
				} else {
					linkStream << "\t\t\t\t\t //  32 bit indices to nodes\n";
				}
			}
			std::ostringstream emitName;
			emitName <<  "NodeTree" << nodeIndex << "Str";
			nodeStream << "0, " << emitName.str() << " - beginLab\n";

			stringTable[ emitName.str() + ":" ] = Core::FilePath( node->name ).RemoveExtension().value();
	}	
	outStream << ".align 8\n" << nodeStream.str();
	outStream << "//---------------------------------------------\n";
	outStream << ".align 8\n" << "beginLab:\n";
	outStream << linkStream.str();

	outStream << "//---------------------------------------------\n";
	outStream << "// string table start\n";
	// now output the string tables
	std::map<std::string,std::string>::const_iterator stIt = stringTable.begin();
	while( stIt != stringTable.end() ) {
		outStream << stIt->first << "\n" << "\"" << stIt->second << "\\0\"" << "\n";
		++stIt;;
	}

	outStream << "endLab:\n";


	// add a Manifest folder to the path
	auto filedir = pOutFilename.DirName();
	filedir = filedir.Append( "Hier" );
	filedir = filedir.Append( pOutFilename.BaseName() );

//#if defined(_DEBUG)
	std::ofstream foutStream;
	auto hietextpath = filedir.ReplaceExtension( ".hietxt" );
	foutStream.open( hietextpath.value().c_str() );
	foutStream << outStream.str();
	foutStream.close();
//#endif

	auto hiepath = filedir.ReplaceExtension( ".hie" );
	foutStream.open( hiepath.value().c_str(), std::ios_base::binary | std::ios_base::out );
	Binify( outStream.str(), foutStream );
	foutStream.close();

	manifest.push_back( Core::ResourceManifestEntry(  HierType, pOutFilename.BaseName().RemoveExtension().value() ) );

	return true;
}

}