/****************************************************************************
*
*                            Open Watcom Project
*
*    Portions Copyright (c) 1983-2002 Sybase, Inc. All Rights Reserved.
*
*  ========================================================================
*
*    This file contains Original Code and/or Modifications of Original
*    Code as defined in and that are subject to the Sybase Open Watcom
*    Public License version 1.0 (the 'License'). You may not use this file
*    except in compliance with the License. BY USING THIS FILE YOU AGREE TO
*    ALL TERMS AND CONDITIONS OF THE LICENSE. A copy of the License is
*    provided with the Original Code and Modifications, and is also
*    available at www.sybase.com/developer/opensource.
*
*    The Original Code and all software distributed under the License are
*    distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
*    EXPRESS OR IMPLIED, AND SYBASE AND ALL CONTRIBUTORS HEREBY DISCLAIM
*    ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR
*    NON-INFRINGEMENT. Please see the License for the specific language
*    governing rights and limitations under the License.
*
*  ========================================================================
*
* Description:  WHEN YOU FIGURE OUT WHAT THIS FILE DOES, PLEASE
*               DESCRIBE IT HERE!
*
****************************************************************************/


#include <wcvector.h>
#include <stdio.h>      // printf debugging
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

#include "chbffile.h"
#include "bind.h"
#include "dialog.h"

Binding::Binding( const char * name )
            : _name( name )
//-----------------------------------
{
    _rectangles = new WCValOrderedVector<Rect>;
    _controls = new WCPtrOrderedVector<Control>;
}

Binding::~Binding()
//-----------------
{
    _rectangles->clear();
    delete _rectangles;

    _controls->clearAndDestroy();
    delete _controls;
}

int Binding::addAbsRelRect( const Rect & r )
//------------------------------------------
{
    _rectangles->append( r );
    return( _rectangles->entries() - 1 );
}

void Binding::addControl( const char * text, const char * id, int absrelrect )
//----------------------------------------------------------------------------
{
    Rect r( (*_rectangles)[ absrelrect ] );
    Control * ctrl;

    ctrl = new Control( text, id, r );
    _controls->insert( ctrl );
}

static int cprintf( CheckedBufferedFile & file, const char * fmt, ... )
//---------------------------------------------------------------------
{
    char    buffer[ 512 ];
    int     len;

    va_list arglist;

    va_start( arglist, fmt );
    len = vsprintf( buffer, fmt, arglist );
    file.write( buffer, strlen( buffer ) );

    va_end( arglist );

    return len;
}

void Binding::writeTimeStamp( CheckedBufferedFile & out )
//-------------------------------------------------------
{
    time_t  timeOfDay;
    char *  tText;

    timeOfDay = time( NULL );
    tText = ctime( &timeOfDay );
    tText[ strlen( tText ) - 1 ] = '\0';        // nuke '\n'

    cprintf( out, "/* %s: generated by dlgprs.exe on %s */\n\n", out.getFileName(), tText );
}

void Binding::bindHeader( const char * hdr )
//------------------------------------------
{
    CheckedBufferedFile header( hdr );
    Control *           ctrl;
    int                 i;

    header.open( CheckedFile::WriteText | O_CREAT, CheckedFile::UserReadWrite );

    cprintf( header, "/* created by dlgprs.exe */\n\n" );

    cprintf( header, "#include \"ctrlrect.h\"\n\n" );

    cprintf( header, "class WWindow;\n\n" );

    cprintf( header, "class %s\n", _name );
    cprintf( header, "{\npublic:\n" );
    cprintf( header, "                              %s( WWindow * prnt );\n", _name );
    cprintf( header, "        virtual               ~%s() {};\n", _name );
    cprintf( header, "        void                  rescale();\n" );
    cprintf( header, "        const ControlRect &   frame() { return __frame; }\n\n" );
    cprintf( header, "protected:\n" );

    cprintf( header, "        WWindow *     __window;\n" );
    cprintf( header, "        ControlRect   __frame;\n\n" );

    for( i = 0; i < _controls->entries(); i += 1 ) {
        ctrl = (*_controls)[ i ];
        cprintf( header, "        ControlRect   %s;\n", ctrl->getText() );
    }

    cprintf( header, "};\n" );

    header.close();
}

static void WriteAbsRel( CheckedBufferedFile & out, Rect & frame, Rect & r, Rect absrel )
//---------------------------------------------------------------------------------------
{
    cprintf( out, "WRect(" );

    if( absrel._x == Absolute ) {
        cprintf( out, "%d, ", r._x );
    } else {
        cprintf( out, "%d, ", -1 * ( r._x - frame._x ) );
    }

    if( absrel._y == Absolute ) {
        cprintf( out, "%d, ", r._y );
    } else {
        cprintf( out, "%d, ", -1 * ( r._y - frame._y ) );
    }

    if( absrel._w == Absolute ) {
        cprintf( out, "%d, ", r._w );
    } else {
        cprintf( out, "%d, ", -1 * ( frame._w - ( r._x + r._w ) ) );
    }

    if( absrel._h == Absolute ) {
        cprintf( out, "%d )", r._h );
    } else {
        cprintf( out, "%d )", -1 * ( frame._h - ( r._y + r._h ) ) );
    }
}

void Binding::bindSource( Dialog * dlg, const char * cpp, const char * hdr )
//--------------------------------------------------------------------------
{
    CheckedBufferedFile source( cpp );
    Control *           dlgControl;
    Control *           ctrl;
    int                 i;
    int                 len;
    int                 trl;
    static char *       dashes = "//------------------------------------------------------------------------------------------";


    source.open( CheckedFile::WriteText | O_CREAT, CheckedFile::UserReadWrite );

    writeTimeStamp( source );

    cprintf( source, "#include <wmetrics.hpp>\n\n" );

    cprintf( source, "#include \"dlgscal.h\"\n" );
    cprintf( source, "#include \"%s\"\n\n", hdr );

    //------------------------- write foo::foo( WWindow * ) --------------------

    len = cprintf( source, "%s::%s( WWindow * prnt )\n", _name, _name );

    trl = cprintf( source, "    : __window( prnt )\n" );
    len = (trl > len) ? trl : len;

    trl = cprintf( source, "    , __frame( WRect(0,0,0,0), \"%s\" )\n", dlg->_caption );
    len = (trl > len) ? trl : len;

    for( i = 0; i < _controls->entries(); i += 1 ) {
        ctrl = (*_controls)[ i ];
        dlgControl = dlg->_controls->find( ctrl );

        if( dlgControl ) {
            Rect r( dlgControl->getRect() );

            trl = cprintf( source, "    , %s( WRect(0,0,0,0), \"%s\" )\n",
                            ctrl->getText(), dlgControl->getText() );
            len = (trl > len) ? trl : len;

        } else {
            printf( "Error: couldn't find dialog control for %s\n", ctrl->getText() );
            exit( EXIT_FAILURE );
        }
    }

    cprintf( source, "%.*s\n", len - 1, dashes );
    cprintf( source, "{\n " );
    cprintf( source, "}\n\n" );

    //-------------------------------------- write foo::rescale() -------------------------------------

    len = cprintf( source, "void %s::rescale()\n", _name );
    cprintf( source, "%.*s\n", len - 1, dashes );
    cprintf( source, "{\n" );
    cprintf( source, "    DialogScaler scl( __window );\n\n" );

    cprintf( source, "    __frame.r = scl.scale( WRect( %d, %d, %d, %d ) );\n",
            dlg->_rect._x, dlg->_rect._y, dlg->_rect._w, dlg->_rect._h );
    cprintf( source, "    __frame.r.w( __frame.r.w() + 2 * WSystemMetrics::dialogFrameWidth() );\n" );
    cprintf( source, "    __frame.r.h( __frame.r.h() + 2 * WSystemMetrics::dialogFrameHeight() \n" );
    cprintf( source, "                + WSystemMetrics::captionSize() );\n" );

    for( i = 0; i < _controls->entries(); i += 1 ) {
        ctrl = (*_controls)[ i ];
        dlgControl = dlg->_controls->find( ctrl );

        if( dlgControl ) {
            Rect r( dlgControl->getRect() );
            cprintf( source, "    %s.r = scl.scale( ", ctrl->getText() );
            WriteAbsRel( source, dlg->_rect, r, ctrl->getRect() );
            cprintf( source, " );\n" );
        } else {
            printf( "Error: couldn't find dialog control for %s\n", ctrl->getText() );
            exit( EXIT_FAILURE );
        }
    }

    cprintf( source, "}\n\n" );

    source.close();
}

void Binding::bind( Dialog * dlg, const char * hdr, const char * cpp )
//--------------------------------------------------------------------
{
    bindHeader( hdr );
    bindSource( dlg, cpp, hdr );
}
