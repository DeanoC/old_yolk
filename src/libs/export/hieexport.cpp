#include "export.h"
#include "scene/hierfile.h"

namespace Export {

bool SaveHierachy( MeshMod::ScenePtr scene, 
                              	Core::ResourceManifestEntryVector& manifest, 
                               	const Core::FilePath pOutFilename ) {
	using namespace Scene;
	std::ostringstream nodeStream;
	std::ostringstream linkStream;
	std::map<std::string,std::string> stringTable;

	// collect nodes
	std::vector< MeshMod::SceneNodePtr > nodeArray;
	std::stack< MeshMod::SceneNodePtr > nodes;

	for( auto nodeIt = scene->sceneNodes.begin(); nodeIt != scene->sceneNodes.end(); ++nodeIt ) {
		nodes.push( *nodeIt );
	}

	while( !nodes.empty() ) {
		MeshMod::SceneNodePtr n = nodes.top();
		nodes.pop(); 
		nodeArray.push_back( n );
		for( uint32_t i=0;i < n->getChildCount();++i ) {
			nodes.push( n->getChild(i) );
		}
	}

	int nodeCounter = 0;
	for( std::vector< MeshMod::SceneNodePtr >::const_iterator it = nodeArray.begin();
		it != nodeArray.end();
		++it ) {
	
		nodeStream << "//---------------------------------------------\n";
		linkStream << "//------------------------------------------\n";
		MeshMod::SceneNodePtr node = *it;
		int nodeIndex = (int) std::distance<std::vector< MeshMod::SceneNodePtr >::const_iterator>( nodeArray.begin(), it );
		std::string nodeName = Core::FilePath( node->name ).RemoveExtension().value();
		std::replace( nodeName.begin(), nodeName.end(), ' ', '_' );
//		CORE_ASSERT( nodeName == node->name ); // this should have be handled by the higher layers now

		std::ostringstream emitName;
		emitName <<  "NodeName" << nodeIndex << "Str";

		if( node->type == "Joint" ) {
			TODO_ASSERT( false && "Bones and joints" );
		} else { // also covers if( node->m_type == "Mesh" ) {
			MeshMod::MeshPtr mesh;
			nodeStream << ".align 8\n";
			if( node->getObjectCount() > 0 ) {
				// TODO what to do with multiple nodes?
				for( unsigned int objNum = 0; objNum < node->getObjectCount();++objNum ) {
					MeshMod::SceneObjectPtr obj = node->getObject(objNum);
					if( objNum > 0 ) {
						nodeStream << "Node_"  << nodeIndex << "_" << objNum << ": \t\t\t\t\t // node Start\n";
					} else {
						nodeStream << "Node_"  << nodeIndex << ": \t\t\t\t\t // node Start\n";
					}
					if( obj->getType() == "Mesh" ) {
						mesh = std::dynamic_pointer_cast<MeshMod::Mesh>( obj );
						nodeStream << "(u16)" << HNT_MESH << "\t\t\t\t\t// type = MESH\n";
					} else {
						nodeStream << "(u16)" << HNT_NODE << "\t\t\t\t\t// type = NODE\n";						
					}
					if( (*it)->properties.size() > 0 ) {
						nodeStream << "(u16) " << HNF_PROPERTIES << "\t\t\t\t\t // flags = HNF_PROPERTIES\n";
					} else {
						nodeStream << "(u16) " << 0 << "\t\t\t\t\t // flags\n";
					}
					nodeStream << ".type float\n";
					// this isn't right for flat hierachy lw scenes, object split need more work
					if( true ) { //objNum > 0 ) {
						nodeStream	<<  (float) node->transform.position[0] << ", "
									<<  (float) node->transform.position[1] << ", "
									<<  (float) node->transform.position[2] << "\t\t\t\t\t// Node Position\n";
						nodeStream	<<  (float) node->transform.orientation[0] << ", "
									<<  (float) node->transform.orientation[1] << ", "
									<<  (float) node->transform.orientation[2] << ", "
									<<  (float) node->transform.orientation[3] << "\t\t\t\t// Node Rotation (quat)\n";
						nodeStream	<<  (float) node->transform.scale[0] << ", "
									<<  (float) node->transform.scale[1] << ", "  
									<<  (float) node->transform.scale[2] << "\t\t\t\t\t// Node Scale\n";
					} else {
						nodeStream	<< "0, 0, 0\t\t\t\t\t// Position same as parent\n";
						nodeStream	<< "0, 0, 0, 1\t\t\t\t// Rotation same as parent\n";
						nodeStream	<< "1, 1, 1\t\t\t\t\t// Scale same as parent\n";
					}
					nodeStream << ".align 8\n";
					nodeStream << ".type u32\n";
					if( obj->getType() == "Mesh" ) {
						std::ostringstream meshLabName;
						meshLabName <<  "MeshName_N" << nodeIndex << "_" << objNum  << "Str";
						nodeStream << "0, " << meshLabName.str() << " - beginLab" << "\t\t\t\t\t// node parameters\n";
						auto baseName = pOutFilename.RemoveExtension().BaseName().value();
						auto meshName = Core::FilePath( baseName + "_" + mesh->getName()).RemoveExtension().value();
						std::replace( meshName.begin(), meshName.end(), ' ', '_' );
						stringTable[ meshLabName.str() + ":" ] = meshName;
					} else {
						nodeStream << "0,0" << "\t\t\t\t\t// node parameters \n";
					}

					if( objNum == 0 ) {
						unsigned int childCount = node->getChildCount() + node->getObjectCount();
						// any children minus this object link it up
						if( childCount > 1 ) {
							nodeStream << "0, NodeTree" << nodeIndex << " - beginLab"<< "\t\t\t\t\t // pointer to children node tree\n";
							linkStream << "NodeTree" << nodeIndex << ":\n";
							linkStream << (uint32_t)(childCount-1) << "\t\t\t\t\t // Number of Childen\n";
							for( unsigned int i=0;i < childCount;++i ) {
								int childNo = (int) std::distance( nodeArray.begin(), std::find( nodeArray.begin(), nodeArray.end(), node->getChild(i) ) );
								if( childCount < node->getChildCount() ) {
									linkStream << "0, Node_" << childNo << " - beginLab\t\t\t\t\t // Pointer to child\n";
								} else {
									if( i == node->getChildCount() ) {
										// skip as this is the node itself
										continue;
									} else {
										// add extra objects attached to this node as children
										// this takes the offset of the child and divides by the size of that
										// child node structure, as this is the same for all nodes, this gives
										// us the out child index
										linkStream << "0, Node_" << childNo << " - beginLab\t\t\t\t\t // Pointer to child\n";
									}
								}
							}
						} else {
							nodeStream << "0,0" << "\t\t\t\t\t// no child linkage\n";
						}
					} else {
						nodeStream << "0,0" << "\t\t\t\t\t// no child linkage\n";
					}

					nodeStream << "0, " << emitName.str() << " - beginLab\n";
					stringTable[ emitName.str() + ":" ] = nodeName;
					if( objNum > 0 ) {
						nodeStream << "Node_"  << nodeIndex << "_" << objNum << "_end: \t\t\t\t\t // node End\n";
					} else {
						nodeStream << "Node_"  << nodeIndex << "_end: \t\t\t\t\t // node End\n";						
					}
					nodeCounter++;
				}	
			} else {
				int childCount = node->getChildCount();
				// skip nodes that have no children. no properties and are identity
				if( childCount == 0 && (*it)->properties.empty() &&
					node->transform.isIdentity() ) {
					continue;
				}
				nodeStream << "Node_"  << nodeIndex << ": \t\t\t\t\t // node Start\n";
				nodeStream << "(u16)" << HNT_NODE << "\t\t\t\t\t// type = NODE\n";
				if( (*it)->properties.size() > 0 ) {
					nodeStream << "(u16) " << HNF_PROPERTIES << "\t\t\t\t\t // flags = HNF_PROPERTIES\n";
				} else {
					nodeStream << "(u16) " << 0 << "\t\t\t\t\t // flags\n";
				}
				nodeStream << ".type float\n";
				nodeStream	<<  (float) node->transform.position[0] << ", "
							<<  (float) node->transform.position[1] << ", "
							<<  (float) node->transform.position[2] << "\t\t\t\t\t// Node Position\n";
				nodeStream	<<  (float) node->transform.orientation[0] << ", "
							<<  (float) node->transform.orientation[1] << ", "
							<<  (float) node->transform.orientation[2] << ", "
							<<  (float) node->transform.orientation[3] << "\t\t\t\t// Node Rotation (quat)\n";
				nodeStream	<<  (float) node->transform.scale[0] << ", "
							<<  (float) node->transform.scale[1] << ", "  
							<<  (float) node->transform.scale[2] << "\t\t\t\t\t// Node Scale\n";
				nodeStream << ".type u32\n";
				nodeStream << ".align 8\n";

				nodeStream << "0,0" << "\t\t\t\t\t// Node parameters ptr\n";
				if( childCount > 0 ) {
					nodeStream << "0, NodeTree" << nodeIndex << " - beginLab"<< "\t\t\t\t\t // pointer to children node tree\n";
					linkStream << "NodeTree" << nodeIndex << ":\n";
					linkStream << (uint32_t) childCount << ", 0\t\t\t\t\t // Number of Childen and alignment dummy\n";
					for( unsigned int i=0;i < (unsigned int) childCount;++i ) {
						int childNo = (int) std::distance( nodeArray.begin(), std::find( nodeArray.begin(), nodeArray.end(), node->getChild(i) ) );
						linkStream << "0, Node_" << childNo << " - beginLab\t\t\t\t\t // Pointer to child\n";
					}
				} else  {
					nodeStream << "0,0" << "\t\t\t\t\t// no linkage\n";
				}

				nodeStream << "0, " << emitName.str() << " - beginLab\n";
				stringTable[ emitName.str() + ":" ] = nodeName;
				nodeCounter++;
			}
		}	
	}

	std::ostringstream outStream;
	// produce a wob node header 
	outStream << "// hierachy file\n";
	outStream << ".type u32\n";
	outStream << HieType << "\t// HIER\n";	// magic
	outStream << "(u16) " << nodeCounter << "\t\t\t\t\t // num nodes\n";
	outStream << "(u8) " << (int)HierVersion << "\t\t\t\t\t // version\n";
	if( scene->properties.size() > 0 ) {
		outStream << "(u8) " << HFF_ENVIRONMENT << "\t\t\t\t\t // flags = HFF_ENVIRONMENT\n";
	} else {
		outStream << "(u8) " << 0 << "\t\t\t\t\t // flags\n";
	}
	outStream << "endLab - beginLab \t\t\t\t\t // total size\n";
	outStream << "//---------------------------------------------\n";
	outStream << "beginLab:\n";
	outStream << "//---------------------------------------------\n";
	outStream << nodeStream.str();
	outStream << "//---------------------------------------------\n";
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

	manifest.push_back( Core::ResourceManifestEntry( HieType, pOutFilename.BaseName().RemoveExtension().value() ) );

	return true;
}

}