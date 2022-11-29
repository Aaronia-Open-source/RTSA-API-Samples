#include <AARTSAAPIWrapper.h>

#include <fmt/core.h>

#include <cstdint>
#include <cstdlib>

namespace AAR = AARTSAAPI;

void walkConfigTree( AAR::ConfigNode &node, int depth = 0 )
{
    std::string shift( depth * 2, ' ' );

    switch ( node.getType() )
    {
    case AAR::ConfigNodeType::GROUP: {
        fmt::print( "{} - {}:\n", shift, node.getName() );
        auto children = node.getChildren();
        for ( auto &child : children )
            walkConfigTree( child, depth + 1 );

        break;
    }

    case AAR::ConfigNodeType::NUMBER:
        fmt::print( "{} - {}: {}\n", shift, node.getName(), node.getFloat() );
        break;

    case AAR::ConfigNodeType::BOOL:
        fmt::print( "{} - {}: {}\n", shift, node.getName(), node.getBool() );
        break;

    case AAR::ConfigNodeType::STRING:
        fmt::print( "{} - {}: {}\n", shift, node.getName(), node.getString() );
        break;

    case AAR::ConfigNodeType::ENUM:
        fmt::print( "{} - {}: {}\n", shift, node.getName(), node.getString() );
        break;

    default:
        fmt::print( "{} - {}: Unhandled type\n", shift, node.getName() );
        break;
    }
}

int main()
{
    auto api = AAR::RTSAWrapper::create( AAR::MemoryMode::MEDIUM );
    auto device = api->getDevice( AAR::DeviceType::SPECTRANV6 );
    device->open( AAR::DeviceMode::RAW );

    fmt::print( "\nWalking config tree:\n" );
    auto configRoot = device->getConfigRoot();
    walkConfigTree( configRoot );

    fmt::print( "\n\nWalking health tree:\n" );
    auto healthRoot = device->getHealthRoot();
    walkConfigTree( healthRoot );

    fmt::print( "\n" );

    return EXIT_SUCCESS;
}
