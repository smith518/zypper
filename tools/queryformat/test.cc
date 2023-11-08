#include <iostream>

#include "Parser.h"

#include <zypp/ResPool.h>
#include <zypp/base/Exception.h>
#include <zypp/misc/DefaultLoadSystem.h>

using namespace zypp;

//////////////////////////////////////////////////////////////////
namespace zypp::qf
{
  template <class ItemT>
  struct Attribute
  {
    bool empty() const;
    unsigned size() const;

  };


  template <class ItemT>
  using AttrRenderer=std::function<void(const ItemT &,std::ostream &)>;

  namespace
  {
    template <class ItemT>
    struct RenderToken
    {
      RenderToken( const Format::TokenPtr & tokenPtr_r )
      : _tokenPtr { tokenPtr_r }
      {}

      bool empty() const;
      size_t size() const;

      Format::TokenPtr _tokenPtr;
    };

    template <class ItemT>
    void render( const String & string_r, const ItemT & item_r, std::ostream & str )
    { str << string_r.value; }


    template <class ItemT>
    void render( const Tag & tag_r, const ItemT & item_r, std::ostream & str )
    { str << tag_r.name; }


    template <class ItemT>
    void render( const Array & array_r, const ItemT & item_r, std::ostream & str )
    { ; }


    template <class ItemT>
    void render( const Conditional & conditional_r, const ItemT & item_r, std::ostream & str )
    { ; }


    template <class ItemT>
    void render( const Format & format_r, const ItemT & item_r, std::ostream & str )
    {
      for ( const auto & el : format_r.tokens ) {
        switch ( el->_type )
        {
          case TokenType::String:
            render( static_cast<const String &>(*el), item_r, str );
            break;
          case TokenType::Tag:
            render( static_cast<const Tag &>(*el), item_r, str );
            break;
          case TokenType::Array:
            render( static_cast<const Array &>(*el), item_r, str );
            break;
          case TokenType::Conditional:
            render( static_cast<const Conditional &>(*el), item_r, str );
            break;
        }
      }
    }
  } // namespace

  template <class ItemT>
  struct Renderer
  {
    Renderer( Format && format_r )
    : _format { std::move(format_r) }
    {}

    Renderer( std::string_view qf_r )
    : _format { qf::parse( qf_r ) }
    {}

    void operator()( const ItemT & item_r ) const
    { operator()( item_r, DOUT ); }

    void operator()( const ItemT & item_r, std::ostream & str ) const
    { render( _format, item_r, str ); }

  private:

    //ctor: Convert Format -> RenderToken[]

    Format _format;
  };

} // namespace zypp::qf


///////////////////////////////////////////////////////////////////
void process( std::string_view qf_r )
try {
  qf::Renderer<PoolItem> render { qf_r };

  static const bool once __attribute__ ((__unused__)) = [](){
    zypp::misc::defaultLoadSystem( misc::LS_NOREFRESH | misc::LS_NOREPOS  );
    return true;
  }();

  unsigned max = 10;
  for ( const auto & el : zypp::ResPool::instance() ) {
    if ( not max-- ) break;
    render(el);
  }
}
catch( const std::exception & ex ) {
  MOUT << "Oopsed: " << ex.what() << endl;
}

///////////////////////////////////////////////////////////////////
int main( int argc, const char ** argv )
{
  using namespace zypp;
  using namespace std::literals;
  ++argv,--argc;
  if ( argc ) {
    if ( *argv == "p"sv ) {
      ++argv,--argc;
      for( ; argc; ++argv,--argc ) {
        MOUT << qf::parse( *argv ) << endl;
      }
      return 22;
    }

    process( "\"%{name}-%{version}-%{release}.%{arch}\"\n" );
    return 0;
    for( ; argc; ++argv,--argc ) {
      process( std::string_view(*argv) );
    }
  }
  else {
    return qf::test::parsetest() == 0 ? 0 : 13;
  }

  return 0;
}


