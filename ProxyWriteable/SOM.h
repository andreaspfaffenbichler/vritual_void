#pragma once

#include <map>
#include <typeindex>
#include <stdexcept>
#include <functional>

namespace BitFactory::simple_open_method
{
	class error : std::runtime_error
	{
		using std::runtime_error::runtime_error;
	};
	template< typename R, typename... ARGS >
	class declare
	{
		using erased_function_t = std::function< R( void*, ARGS... ) >;
		std::map< std::type_index, erased_function_t > methodTable_; 
	public:
		using erased_function_t = std::function< R(void*, ARGS... ) >;
		auto define_erased( const std::type_info& register_type_info, const std::function< R( void*, ARGS... ) >& f )
		{
			auto entry = methodTable_.find( register_type_info );
			if( entry != methodTable_.end() )
				throw error( "Method for type already registered." );
			methodTable_[ register_type_info ] = f;
			return definition{};
		}
		template< typename SELF >
		auto define( const std::function< R( SELF*, ARGS... ) >& f )
		{
			return define_erased( typeid( SELF ), [ f ]( void* self, ARGS... args )->R
			{
				return f( reinterpret_cast< SELF* >( self ), std::forward< ARGS >( args )... );
			});
		}
		template< typename SELF >
		R operator()( SELF* self, ARGS&&... args ) const
		{
			return (*this)( typeid( SELF ), self, std::forward< ARGS >( args )... );
		}
		R operator()( const std::type_info& type_info, void* self, ARGS&&... args ) const
		{
			auto entry = methodTable_.find( type_info );
			if( entry == methodTable_.end() )
				throw error( "No registered method." );
			return entry->second( self, std::forward< ARGS >( args )... );
		}	

		class definition{};
	};


}