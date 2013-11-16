 /*
 www.sourceforge.net/projects/tinyxml
 Original code (2.0 and earlier )copyright (c) 2000-2002 Lee Thomason (www.grinninglizard.com)
 
 This software is provided 'as-is', without any express or implied
 warranty. In no event will the authors be held liable for any
 damages arising from the use of this software.
 
 Permission is granted to anyone to use this software for any
 purpose, including commercial applications, and to alter it and
 redistribute it freely, subject to the following restrictions:
 
 1. The origin of this software must not be misrepresented; you must
 not claim that you wrote the original software. If you use this
 software in a product, an acknowledgment in the product documentation
 would be appreciated but is not required.
 
 2. Altered source versions must be plainly marked as such, and
 must not be misrepresented as being the original software.
 
 3. This notice may not be removed or altered from any source
 distribution.
 */
 
 
 #ifndef TINYXML_INCLUDED
 #define TINYXML_INCLUDED
 
 #ifdef _MSC_VER
 #pragma warning( disable : 4530 )
 #pragma warning( disable : 4786 )
 #endif
 
 #include <ctype.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <assert.h>
 
 // Help out windows:
 #if defined( _DEBUG ) && !defined( DEBUG )
 #define DEBUG
 #endif
 
 #if defined( DEBUG ) && defined( _MSC_VER )
 #include <windows.h>
 #define TIXML_LOG OutputDebugString
 #else
 #define TIXML_LOG printf
 #endif
 
 #ifdef TIXML_USE_STL
     #include <string>
     #include <iostream>
     //#include <ostream>
     #define TIXML_STRING    std::string
     #define TIXML_ISTREAM   std::istream
     #define TIXML_OSTREAM   std::ostream
 #else
     #include "tinystr.h"
     #define TIXML_STRING    TiXmlString
     #define TIXML_OSTREAM   TiXmlOutStream
 #endif
 
 class TiXmlDocument;
 class TiXmlElement;
 class TiXmlComment;
 class TiXmlUnknown;
 class TiXmlAttribute;
 class TiXmlText;
 class TiXmlDeclaration;
 
 class TiXmlParsingData;
 
 /*  Internal structure for tracking location of items 
     in the XML file.
 */
 struct TiXmlCursor
 {
     TiXmlCursor()       { Clear(); }
     void Clear()        { row = col = -1; }
 
     int row;    // 0 based.
     int col;    // 0 based.
 };
 
 
 // Only used by Attribute::Query functions
 enum 
 { 
     TIXML_SUCCESS,
     TIXML_NO_ATTRIBUTE,
     TIXML_WRONG_TYPE
 };
 
 class TiXmlBase
 {
     friend class TiXmlNode;
     friend class TiXmlElement;
     friend class TiXmlDocument;
 
 public:
     TiXmlBase()                             {userData = 0;}
     virtual ~TiXmlBase()                    {}
 
     virtual void Print( FILE* cfile, int depth ) const = 0;
 
     static void SetCondenseWhiteSpace( bool condense )      { condenseWhiteSpace = condense; }
 
     static bool IsWhiteSpaceCondensed()                     { return condenseWhiteSpace; }
 
     int Row() const         { return location.row + 1; }
     int Column() const      { return location.col + 1; }    
 
     void  SetUserData( void* user )         { userData = user; }
     void* GetUserData()                     { return userData; }
 
     // Table that returs, for a given lead byte, the total number of bytes
     // in the UTF-8 sequence.
     static const int utf8ByteTable[256];
 
 protected:
 
     // Store encoding id
     enum EEncodings
     {
         UTF_8 = 0,
         windows_1251
     };
 
     static EEncodings   docEncoding;
 
     // See STL_STRING_BUG
     // Utility class to overcome a bug.
     class StringToBuffer
     {
       public:
         StringToBuffer( const TIXML_STRING& str );
         ~StringToBuffer();
         char* buffer;
     };
 
     static const char*  SkipWhiteSpace( const char* );
     inline static bool  IsWhiteSpace( char c )      
     { 
         return ( isspace( (unsigned char) c ) || c == '\n' || c == '\r' ); 
     }
 
     virtual void StreamOut (TIXML_OSTREAM *) const = 0;
 
     #ifdef TIXML_USE_STL
         static bool StreamWhiteSpace( TIXML_ISTREAM * in, TIXML_STRING * tag );
         static bool StreamTo( TIXML_ISTREAM * in, int character, TIXML_STRING * tag );
     #endif
 
     /*  Reads an XML name into the string provided. Returns
         a pointer just past the last character of the name,
         or 0 if the function has an error.
     */
     static const char* ReadName( const char* p, TIXML_STRING* name );
 
     /*  Reads text. Returns a pointer past the given end tag.
         Wickedly complex options, but it keeps the (sensitive) code in one place.
     */
     static const char* ReadText(    const char* in,             // where to start
                                     TIXML_STRING* text,         // the string read
                                     bool ignoreWhiteSpace,      // whether to keep the white space
                                     const char* endTag,         // what ends this text
                                     bool ignoreCase );          // whether to ignore case in the end tag
 
     virtual const char* Parse( const char* p, TiXmlParsingData* data ) = 0;
 
     // If an entity has been found, transform it into a character.
     static const char* GetEntity( const char* in, char* value, int* length );
 
     // Get a character, while interpreting entities.
     // The length can be from 0 to 4 bytes.
     inline static const char* GetCharUTF8( const char* p, char* _value, int* length )
     {
         assert( p );
         *length = utf8ByteTable[ *((unsigned char*)p) ];
         assert( *length >= 0 && *length < 5 );
 
         if ( *length == 1 )
         {
             if ( *p == '&' )
                 return GetEntity( p, _value, length );
             *_value = *p;
             return p+1;
         }
         else if ( *length )
         {
             strncpy( _value, p, *length );
             return p + (*length);
         }
         else
         {
             // Not valid text.
             return 0;
         }
     }
 
     // Puts a string to a stream, expanding entities as it goes.
     // Note this should not contian the '<', '>', etc, or they will be transformed into entities!
     static void PutString( const TIXML_STRING& str, TIXML_OSTREAM* out );
 
     static void PutString( const TIXML_STRING& str, TIXML_STRING* out );
 
     // Return true if the next characters in the stream are any of the endTag sequences.
     // Ignore case only works for english, and should only be relied on when comparing
     // to Engilish words: StringEqual( p, "version", true ) is fine.
     static bool StringEqual(    const char* p,
                                 const char* endTag,
                                 bool ignoreCase );
 
 
     enum
     {
         TIXML_NO_ERROR = 0,
         TIXML_ERROR,
         TIXML_ERROR_OPENING_FILE,
         TIXML_ERROR_OUT_OF_MEMORY,
         TIXML_ERROR_PARSING_ELEMENT,
         TIXML_ERROR_FAILED_TO_READ_ELEMENT_NAME,
         TIXML_ERROR_READING_ELEMENT_VALUE,
         TIXML_ERROR_READING_ATTRIBUTES,
         TIXML_ERROR_PARSING_EMPTY,
         TIXML_ERROR_READING_END_TAG,
         TIXML_ERROR_PARSING_UNKNOWN,
         TIXML_ERROR_PARSING_COMMENT,
         TIXML_ERROR_PARSING_DECLARATION,
         TIXML_ERROR_DOCUMENT_EMPTY,
 
         TIXML_ERROR_STRING_COUNT
     };
     static const char* errorString[ TIXML_ERROR_STRING_COUNT ];
 
     TiXmlCursor location;
 
     void*           userData;
     
     // None of these methods are reliable for any language except English.
     // Good for approximation, not great for accuracy.
     static int IsAlphaUTF8( unsigned char anyByte );
     static int IsAlphaNumUTF8( unsigned char anyByte );
     inline static int ToLowerUTF8( int v )
     {
         if ( v < 128 ) return tolower( v );
         return v;
     }
     static void ConvertUTF32ToUTF8( unsigned long input, char* output, int* length );
 
 private:
     TiXmlBase( const TiXmlBase& );              // not implemented.
     void operator=( const TiXmlBase& base );    // not allowed.
 
     struct Entity
     {
         const char*     str;
         unsigned int    strLength;
         char            chr;
     };
     enum
     {
         NUM_ENTITY = 5,
         MAX_ENTITY_LENGTH = 6
 
     };
     static Entity entity[ NUM_ENTITY ];
     static bool condenseWhiteSpace;
 };
 
 
 class TiXmlNode : public TiXmlBase
 {
     friend class TiXmlDocument;
     friend class TiXmlElement;
 
 public:
     #ifdef TIXML_USE_STL    
 
         friend std::istream& operator >> (std::istream& in, TiXmlNode& base);
 
         friend std::ostream& operator<< (std::ostream& out, const TiXmlNode& base);
 
         friend std::string& operator<< (std::string& out, const TiXmlNode& base );
 
     #else
         // Used internally, not part of the public API.
         friend TIXML_OSTREAM& operator<< (TIXML_OSTREAM& out, const TiXmlNode& base);
     #endif
 
     enum NodeType
     {
         DOCUMENT,
         ELEMENT,
         COMMENT,
         UNKNOWN,
         TEXT,
         DECLARATION,
         TYPECOUNT
     };
 
     virtual ~TiXmlNode();
 
     const char * Value() const { return value.c_str (); }
 
     void SetValue(const char * _value) { value = _value;}
 
     #ifdef TIXML_USE_STL
 
     void SetValue( const std::string& _value )    
     {     
         StringToBuffer buf( _value );
         SetValue( buf.buffer ? buf.buffer : "" );       
     }   
     #endif
 
     void Clear();
 
     TiXmlNode* Parent() const                   { return parent; }
 
     TiXmlNode* FirstChild() const   { return firstChild; }      
     TiXmlNode* FirstChild( const char * value ) const;          
 
     TiXmlNode* LastChild() const    { return lastChild; }       
     TiXmlNode* LastChild( const char * value ) const;           
 
     #ifdef TIXML_USE_STL
     TiXmlNode* FirstChild( const std::string& _value ) const    {   return FirstChild (_value.c_str ());    }   
     TiXmlNode* LastChild( const std::string& _value ) const     {   return LastChild (_value.c_str ()); }   
     #endif
 
     TiXmlNode* IterateChildren( TiXmlNode* previous ) const;
 
     TiXmlNode* IterateChildren( const char * value, TiXmlNode* previous ) const;
 
     #ifdef TIXML_USE_STL
     TiXmlNode* IterateChildren( const std::string& _value, TiXmlNode* previous ) const  {   return IterateChildren (_value.c_str (), previous); }   
     #endif
 
     TiXmlNode* InsertEndChild( const TiXmlNode& addThis );
 
 
     TiXmlNode* LinkEndChild( TiXmlNode* addThis );
 
     TiXmlNode* InsertBeforeChild( TiXmlNode* beforeThis, const TiXmlNode& addThis );
 
     TiXmlNode* InsertAfterChild(  TiXmlNode* afterThis, const TiXmlNode& addThis );
 
     TiXmlNode* ReplaceChild( TiXmlNode* replaceThis, const TiXmlNode& withThis );
 
     bool RemoveChild( TiXmlNode* removeThis );
 
     TiXmlNode* PreviousSibling() const          { return prev; }
 
     TiXmlNode* PreviousSibling( const char * ) const;
 
     #ifdef TIXML_USE_STL
     TiXmlNode* PreviousSibling( const std::string& _value ) const   {   return PreviousSibling (_value.c_str ());   }   
     TiXmlNode* NextSibling( const std::string& _value) const        {   return NextSibling (_value.c_str ());   }   
     #endif
 
     TiXmlNode* NextSibling() const              { return next; }
 
     TiXmlNode* NextSibling( const char * ) const;
 
     TiXmlElement* NextSiblingElement() const;
 
     TiXmlElement* NextSiblingElement( const char * ) const;
 
     #ifdef TIXML_USE_STL
     TiXmlElement* NextSiblingElement( const std::string& _value) const  {   return NextSiblingElement (_value.c_str ());    }   
     #endif
 
     TiXmlElement* FirstChildElement()   const;
 
     TiXmlElement* FirstChildElement( const char * value ) const;
 
     #ifdef TIXML_USE_STL
     TiXmlElement* FirstChildElement( const std::string& _value ) const  {   return FirstChildElement (_value.c_str ()); }   
     #endif
 
     virtual int Type() const    { return type; }
 
     TiXmlDocument* GetDocument() const;
 
     bool NoChildren() const                     { return !firstChild; }
 
     TiXmlDocument* ToDocument() const       { return ( this && type == DOCUMENT ) ? (TiXmlDocument*) this : 0; } 
     TiXmlElement*  ToElement() const        { return ( this && type == ELEMENT  ) ? (TiXmlElement*)  this : 0; } 
     TiXmlComment*  ToComment() const        { return ( this && type == COMMENT  ) ? (TiXmlComment*)  this : 0; } 
     TiXmlUnknown*  ToUnknown() const        { return ( this && type == UNKNOWN  ) ? (TiXmlUnknown*)  this : 0; } 
     TiXmlText*     ToText()    const        { return ( this && type == TEXT     ) ? (TiXmlText*)     this : 0; } 
     TiXmlDeclaration* ToDeclaration() const { return ( this && type == DECLARATION ) ? (TiXmlDeclaration*) this : 0; } 
 
     virtual TiXmlNode* Clone() const = 0;
 
 protected:
     TiXmlNode( NodeType _type );
 
     #ifdef TIXML_USE_STL
         // The real work of the input operator.
         virtual void StreamIn( TIXML_ISTREAM* in, TIXML_STRING* tag ) = 0;
     #endif
 
     // Figure out what is at *p, and parse it. Returns null if it is not an xml node.
     TiXmlNode* Identify( const char* start );
     void CopyToClone( TiXmlNode* target ) const { target->SetValue (value.c_str() );
                                                   target->userData = userData; }
 
     // Internal Value function returning a TIXML_STRING
     const TIXML_STRING& SValue() const  { return value ; }
 
     TiXmlNode*      parent;
     NodeType        type;
 
     TiXmlNode*      firstChild;
     TiXmlNode*      lastChild;
 
     TIXML_STRING    value;
 
     TiXmlNode*      prev;
     TiXmlNode*      next;
 
 private:
     TiXmlNode( const TiXmlNode& );              // not implemented.
     void operator=( const TiXmlNode& base );    // not allowed.
 };
 
 
 class TiXmlAttribute : public TiXmlBase
 {
     friend class TiXmlAttributeSet;
 
 public:
     TiXmlAttribute() : TiXmlBase()
     {
         document = 0;
         prev = next = 0;
     }
 
     #ifdef TIXML_USE_STL
 
     TiXmlAttribute( const std::string& _name, const std::string& _value )
     {
         name = _name;
         value = _value;
         document = 0;
         prev = next = 0;
     }
     #endif
 
     TiXmlAttribute( const char * _name, const char * _value )
     {
         name = _name;
         value = _value;
         document = 0;
         prev = next = 0;
     }
 
     const char*     Name()  const       { return name.c_str (); }       
     const char*     Value() const       { return value.c_str (); }      
     const int       IntValue() const;                                   
     const double    DoubleValue() const;                                
 
     int QueryIntValue( int* value ) const;
     int QueryDoubleValue( double* value ) const;
 
     void SetName( const char* _name )   { name = _name; }               
     void SetValue( const char* _value ) { value = _value; }             
 
     void SetIntValue( int value );                                      
     void SetDoubleValue( double value );                                
 
     #ifdef TIXML_USE_STL
 
     void SetName( const std::string& _name )    
     {   
         StringToBuffer buf( _name );
         SetName ( buf.buffer ? buf.buffer : "error" );  
     }
     void SetValue( const std::string& _value )  
     {   
         StringToBuffer buf( _value );
         SetValue( buf.buffer ? buf.buffer : "error" );  
     }
     #endif
 
     TiXmlAttribute* Next() const;
     TiXmlAttribute* Previous() const;
 
     bool operator==( const TiXmlAttribute& rhs ) const { return rhs.name == name; }
     bool operator<( const TiXmlAttribute& rhs )  const { return name < rhs.name; }
     bool operator>( const TiXmlAttribute& rhs )  const { return name > rhs.name; }
 
     /*  [internal use]
         Attribtue parsing starts: first letter of the name
                          returns: the next char after the value end quote
     */
     virtual const char* Parse( const char* p, TiXmlParsingData* data );
 
     // [internal use]
     virtual void Print( FILE* cfile, int depth ) const;
 
     virtual void StreamOut( TIXML_OSTREAM * out ) const;
     // [internal use]
     // Set the document pointer so the attribute can report errors.
     void SetDocument( TiXmlDocument* doc )  { document = doc; }
 
 private:
     TiXmlAttribute( const TiXmlAttribute& );                // not implemented.
     void operator=( const TiXmlAttribute& base );   // not allowed.
 
     TiXmlDocument*  document;   // A pointer back to a document, for error reporting.
     TIXML_STRING name;
     TIXML_STRING value;
     TiXmlAttribute* prev;
     TiXmlAttribute* next;
 };
 
 
 /*  A class used to manage a group of attributes.
     It is only used internally, both by the ELEMENT and the DECLARATION.
     
     The set can be changed transparent to the Element and Declaration
     classes that use it, but NOT transparent to the Attribute
     which has to implement a next() and previous() method. Which makes
     it a bit problematic and prevents the use of STL.
 
     This version is implemented with circular lists because:
         - I like circular lists
         - it demonstrates some independence from the (typical) doubly linked list.
 */
 class TiXmlAttributeSet
 {
 public:
     TiXmlAttributeSet();
     ~TiXmlAttributeSet();
 
     void Add( TiXmlAttribute* attribute );
     void Remove( TiXmlAttribute* attribute );
 
     TiXmlAttribute* First() const   { return ( sentinel.next == &sentinel ) ? 0 : sentinel.next; }
     TiXmlAttribute* Last()  const   { return ( sentinel.prev == &sentinel ) ? 0 : sentinel.prev; }
     TiXmlAttribute* Find( const char * name ) const;
 
 private:
     TiXmlAttribute sentinel;
 };
 
 
 class TiXmlElement : public TiXmlNode
 {
 public:
     TiXmlElement (const char * in_value);
 
     #ifdef TIXML_USE_STL
 
     TiXmlElement( const std::string& _value ) :     TiXmlNode( TiXmlNode::ELEMENT )
     {
         firstChild = lastChild = 0;
         value = _value;
     }
     #endif
 
     virtual ~TiXmlElement();
 
     const char* Attribute( const char* name ) const;
 
     const char* Attribute( const char* name, int* i ) const;
 
     const char* Attribute( const char* name, double* d ) const;
 
     int QueryIntAttribute( const char* name, int* value ) const;
     int QueryDoubleAttribute( const char* name, double* value ) const;
 
     void SetAttribute( const char* name, const char * value );
 
     #ifdef TIXML_USE_STL
     const char* Attribute( const std::string& name ) const              { return Attribute( name.c_str() ); }
     const char* Attribute( const std::string& name, int* i ) const      { return Attribute( name.c_str(), i ); }
 
     void SetAttribute( const std::string& name, const std::string& _value ) 
     {   
         StringToBuffer n( name );
         StringToBuffer v( _value );
         if ( n.buffer && v.buffer )
             SetAttribute (n.buffer, v.buffer ); 
     }   
     void SetAttribute( const std::string& name, int _value )    
     {   
         StringToBuffer n( name );
         if ( n.buffer )
             SetAttribute (n.buffer, _value);    
     }   
     #endif
 
     void SetAttribute( const char * name, int value );
 
     void SetDoubleAttribute( const char * name, double value );
 
     void RemoveAttribute( const char * name );
     #ifdef TIXML_USE_STL
     void RemoveAttribute( const std::string& name ) {   RemoveAttribute (name.c_str ());    }   
     #endif
 
     TiXmlAttribute* FirstAttribute() const  { return attributeSet.First(); }        
     TiXmlAttribute* LastAttribute() const   { return attributeSet.Last(); }     
 
     // [internal use] Creates a new Element and returs it.
     virtual TiXmlNode* Clone() const;
     // [internal use]
 
     virtual void Print( FILE* cfile, int depth ) const;
 
 protected:
 
     // Used to be public [internal use]
     #ifdef TIXML_USE_STL
         virtual void StreamIn( TIXML_ISTREAM * in, TIXML_STRING * tag );
     #endif
     virtual void StreamOut( TIXML_OSTREAM * out ) const;
 
     /*  [internal use]
         Attribtue parsing starts: next char past '<'
                          returns: next char past '>'
     */
     virtual const char* Parse( const char* p, TiXmlParsingData* data );
 
     /*  [internal use]
         Reads the "value" of the element -- another element, or text.
         This should terminate with the current end tag.
     */
     const char* ReadValue( const char* in, TiXmlParsingData* prevData );
 
 private:
     TiXmlElement( const TiXmlElement& );                // not implemented.
     void operator=( const TiXmlElement& base ); // not allowed.
 
     TiXmlAttributeSet attributeSet;
 };
 
 
 class TiXmlComment : public TiXmlNode
 {
 public:
     TiXmlComment() : TiXmlNode( TiXmlNode::COMMENT ) {}
 
     virtual ~TiXmlComment() {}
 
     // [internal use] Creates a new Element and returs it.
     virtual TiXmlNode* Clone() const;
     // [internal use]
     virtual void Print( FILE* cfile, int depth ) const;
 protected:
     // used to be public
     #ifdef TIXML_USE_STL
         virtual void StreamIn( TIXML_ISTREAM * in, TIXML_STRING * tag );
     #endif
     virtual void StreamOut( TIXML_OSTREAM * out ) const;
     /*  [internal use]
         Attribtue parsing starts: at the ! of the !--
                          returns: next char past '>'
     */
     virtual const char* Parse( const char* p, TiXmlParsingData* data );
 
 private:
     TiXmlComment( const TiXmlComment& );                // not implemented.
     void operator=( const TiXmlComment& base ); // not allowed.
 
 };
 
 
 class TiXmlText : public TiXmlNode
 {
     friend class TiXmlElement;
 public:
     TiXmlText (const char * initValue) : TiXmlNode (TiXmlNode::TEXT)
     {
         SetValue( initValue );
     }
     virtual ~TiXmlText() {}
 
     #ifdef TIXML_USE_STL
 
     TiXmlText( const std::string& initValue ) : TiXmlNode (TiXmlNode::TEXT)
     {
         SetValue( initValue );
     }
     #endif
 
     // [internal use]
     virtual void Print( FILE* cfile, int depth ) const;
 
 protected :
     // [internal use] Creates a new Element and returns it.
     virtual TiXmlNode* Clone() const;
     virtual void StreamOut ( TIXML_OSTREAM * out ) const;
     // [internal use]
     bool Blank() const; // returns true if all white space and new lines
     /*  [internal use]
             Attribtue parsing starts: First char of the text
                              returns: next char past '>'
     */
     virtual const char* Parse( const char* p, TiXmlParsingData* data );
     // [internal use]
     #ifdef TIXML_USE_STL
         virtual void StreamIn( TIXML_ISTREAM * in, TIXML_STRING * tag );
     #endif
 
 private:
     TiXmlText( const TiXmlText& );              // not implemented.
     void operator=( const TiXmlText& base );    // not allowed.
 };
 
 
 class TiXmlDeclaration : public TiXmlNode
 {
 public:
     TiXmlDeclaration()   : TiXmlNode( TiXmlNode::DECLARATION ) {}
 
 #ifdef TIXML_USE_STL
 
     TiXmlDeclaration(   const std::string& _version,
                         const std::string& _encoding,
                         const std::string& _standalone )
             : TiXmlNode( TiXmlNode::DECLARATION )
     {
         version = _version;
         encoding = _encoding;
         standalone = _standalone;
     }
 #endif
 
     TiXmlDeclaration(   const char* _version,
                         const char* _encoding,
                         const char* _standalone );
 
     virtual ~TiXmlDeclaration() {}
 
     const char * Version() const        { return version.c_str (); }
     const char * Encoding() const       { return encoding.c_str (); }
     const char * Standalone() const     { return standalone.c_str (); }
 
     // [internal use] Creates a new Element and returs it.
     virtual TiXmlNode* Clone() const;
     // [internal use]
     virtual void Print( FILE* cfile, int depth ) const;
 
 protected:
     // used to be public
     #ifdef TIXML_USE_STL
         virtual void StreamIn( TIXML_ISTREAM * in, TIXML_STRING * tag );
     #endif
     virtual void StreamOut ( TIXML_OSTREAM * out) const;
     //  [internal use]
     //  Attribtue parsing starts: next char past '<'
     //                   returns: next char past '>'
 
     virtual const char* Parse( const char* p, TiXmlParsingData* data );
 
 private:
     TiXmlDeclaration( const TiXmlDeclaration& copy );
     void operator=( const TiXmlDeclaration& copy );
 
     TIXML_STRING version;
     TIXML_STRING encoding;
     TIXML_STRING standalone;
 };
 
 
 class TiXmlUnknown : public TiXmlNode
 {
 public:
     TiXmlUnknown() : TiXmlNode( TiXmlNode::UNKNOWN ) {}
     virtual ~TiXmlUnknown() {}
 
     // [internal use]
     virtual TiXmlNode* Clone() const;
     // [internal use]
     virtual void Print( FILE* cfile, int depth ) const;
 protected:
     #ifdef TIXML_USE_STL
         virtual void StreamIn( TIXML_ISTREAM * in, TIXML_STRING * tag );
     #endif
     virtual void StreamOut ( TIXML_OSTREAM * out ) const;
     /*  [internal use]
         Attribute parsing starts: First char of the text
                          returns: next char past '>'
     */
     virtual const char* Parse( const char* p, TiXmlParsingData* data );
 
 private:
     TiXmlUnknown( const TiXmlUnknown& copy );
     void operator=( const TiXmlUnknown& copy );
 
 };
 
 
 class TiXmlDocument : public TiXmlNode
 {
 public:
     TiXmlDocument();
     TiXmlDocument( const char * documentName );
 
     #ifdef TIXML_USE_STL
 
     TiXmlDocument( const std::string& documentName ) :
         TiXmlNode( TiXmlNode::DOCUMENT )
     {
         value = documentName;
         error = false;
     }
     #endif
 
     virtual ~TiXmlDocument() {}
 
     bool LoadFile();
     bool SaveFile() const;
     bool LoadFile( const char * filename );
     bool SaveFile( const char * filename ) const;
 
     #ifdef TIXML_USE_STL
     bool LoadFile( const std::string& filename )            
     {
         StringToBuffer f( filename );
         return ( f.buffer && LoadFile( f.buffer ));
     }
     bool SaveFile( const std::string& filename ) const      
     {
         StringToBuffer f( filename );
         return ( f.buffer && SaveFile( f.buffer ));
     }
     #endif
 
     virtual const char* Parse( const char* p, TiXmlParsingData* data = 0 );
 
     TiXmlElement* RootElement() const       { return FirstChildElement(); }
 
     bool Error() const                      { return error; }
 
     const char * ErrorDesc() const  { return errorDesc.c_str (); }
 
     const int ErrorId() const               { return errorId; }
 
     int ErrorRow()  { return errorLocation.row+1; }
     int ErrorCol()  { return errorLocation.col+1; } 
 
     void SetTabSize( int _tabsize )     { tabsize = _tabsize; }
 
     int TabSize() const { return tabsize; }
 
     void ClearError()                       {   error = false; 
                                                 errorId = 0; 
                                                 errorDesc = ""; 
                                                 errorLocation.row = errorLocation.col = 0; 
                                                 //errorLocation.last = 0; 
                                             }
 
     void Print() const                      { Print( stdout, 0 ); }
 
     // [internal use]
     virtual void Print( FILE* cfile, int depth = 0 ) const;
     // [internal use]
     void SetError( int err, const char* errorLocation, TiXmlParsingData* prevData );
 
 protected :
     virtual void StreamOut ( TIXML_OSTREAM * out) const;
     // [internal use]
     virtual TiXmlNode* Clone() const;
     #ifdef TIXML_USE_STL
         virtual void StreamIn( TIXML_ISTREAM * in, TIXML_STRING * tag );
     #endif
 
 private:
     TiXmlDocument( const TiXmlDocument& copy );
     void operator=( const TiXmlDocument& copy );
 
     bool error;
     int  errorId;
     TIXML_STRING errorDesc;
     int tabsize;
     TiXmlCursor errorLocation;
 };
 
 
 class TiXmlHandle
 {
 public:
     TiXmlHandle( TiXmlNode* node )                  { this->node = node; }
     TiXmlHandle( const TiXmlHandle& ref )           { this->node = ref.node; }
     TiXmlHandle operator=( const TiXmlHandle& ref ) { this->node = ref.node; return *this; }
 
     TiXmlHandle FirstChild() const;
     TiXmlHandle FirstChild( const char * value ) const;
     TiXmlHandle FirstChildElement() const;
     TiXmlHandle FirstChildElement( const char * value ) const;
 
     TiXmlHandle Child( const char* value, int index ) const;
     TiXmlHandle Child( int index ) const;
     TiXmlHandle ChildElement( const char* value, int index ) const;
     TiXmlHandle ChildElement( int index ) const;
 
     #ifdef TIXML_USE_STL
     TiXmlHandle FirstChild( const std::string& _value ) const               { return FirstChild( _value.c_str() ); }
     TiXmlHandle FirstChildElement( const std::string& _value ) const        { return FirstChildElement( _value.c_str() ); }
 
     TiXmlHandle Child( const std::string& _value, int index ) const         { return Child( _value.c_str(), index ); }
     TiXmlHandle ChildElement( const std::string& _value, int index ) const  { return ChildElement( _value.c_str(), index ); }
     #endif
 
     TiXmlNode* Node() const         { return node; } 
     TiXmlElement* Element() const   { return ( ( node && node->ToElement() ) ? node->ToElement() : 0 ); }
     TiXmlText* Text() const         { return ( ( node && node->ToText() ) ? node->ToText() : 0 ); }
     TiXmlUnknown* Unknown() const           { return ( ( node && node->ToUnknown() ) ? node->ToUnknown() : 0 ); }
 
 private:
     TiXmlNode* node;
 };
 
 
 #endif
 




