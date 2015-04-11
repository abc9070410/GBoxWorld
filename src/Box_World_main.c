/*
GTK��@��²���ܮw�f�����C���C
�C�����|�p����c���ƩM���ʦ��ƨ���ܦb�W��label��r���C
�H��L����V�䱱��C�������ܮw�u�H�A�i��h�B�u�@�C 
��space��i�h�^�e�@�B�A�w�]���̦h�h�@�ʨB�C
�H32x32���h�i���ɧ@���a�Ϥ���Aø�s�X��i�a�ϡC 
�iŪ���a���ɡA�ΨϥΤ��ئa�ϡA ���ئa�Ϧ����i�A�i�C���������󴫦a�ϡC 
�h�ưT���|��ܦb�W�誺entry��J�ءC 

update: 2010.5.15
*/ 


#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <stdlib.h>
#include <string.h>
#include <gdk/gdkkeysyms.h>

 
#define EQU ==
#define WIDTH 17  // �e 
#define HEIGHT 12 // �� 
#define COUNT_OF_RECORD 100 // �i�^���X�B   

 
/*
 1. Ū���a���� ok
 2. ���w�C���d�� ok
   2.1. ���V road or target -> OK
        ���V box -> �ݧP�_box�P�Ӥ�V�O���O��road or target�A�Y�O -> OK 
 3. �]�w���ʽc�l������ ok
 4. ��w�C������������ ok
 5. �iundo�ܦh�B(���s+�ֱ���) ok
    5.1. undo�ɡA�P���٭���ʪ��c�l���� ok
    5.2. �q�T�w���Ƴ]���i�`���C�_�h�W�L���ƴN�|error ok
 6. �������ʽc�l������ ok
 7. ���ئa���� ok
 8. ������i�������U�i�a�϶i��C�� ok 
 9. �]�pAI 
*/ 


enum directionType { // �|�ؤ�V�G�W�B�U�B���B�k 
    right ,
    left ,
    up ,
    down
} ;

typedef enum directionType Dt ;


enum mapType {
    workman , // �ܮw�u�H
    box ,     // �n�h���c�l
    boxOpen , // �c�l��b�ؼФW�᪺���A
    target ,  // �c�l�n�������ؼ�
    road , // �i�樫���a�O
    background , // ����H�~���a��
    wall , // ���
    none ,  // �S���F��
    other   // ���`�����Ӧ��o�تF��~��......debug��
} ;

typedef enum mapType Mt ;


struct undoData {
    int workmanX ;
    int workmanY ;
    int countOfPushBox ;
    Mt mapType[HEIGHT][WIDTH] ;  
    //GtkWidget *image[HEIGHT][WIDTH] ;      
} ;

typedef struct undoData Ud ;


GtkWidget *gLabel[HEIGHT][WIDTH] ;
GtkWidget *gImage[HEIGHT][WIDTH] ;
Mt gMapType[HEIGHT][WIDTH] ; 
Mt gMapTypeTemp[HEIGHT][WIDTH] ; // �٭�� 
Ud gUndoData[COUNT_OF_RECORD] ; // �٭�� 

  
int gWorkmanX = 5 ;  // �w�]�ܮw�u�H����m 
int gWorkmanY = 5 ;  

int gCountOfPushBox = 0 ; // ���ʽc�l������ 
int gUndoNow = 0 ; // �ثe�O���X���a�ϤF 
int gUndoNowInArrary = 0 ; // ���arrary����gUndoNow���� 
int gUndoMaxValue = 0 ; // gUndoNow�̰��F�쪺�ȡA�u�|�[�A���|��C 
  
// �w�]���a����  
const char *mapFile = "_map5.txt" ;  

// ���Ψ쪺�X�ӹϤ���
const char *workmanImageFile = "_rc_workman.png" ;
const char *boxImageFile = "_rc_box.png" ;
const char *boxOpenImageFile = "_rc_boxOpen.png" ;
const char *targetImageFile = "_rc_target.png" ;
const char *roadImageFile = "_rc_road.jpg" ;
const char *backgroundImageFile = "_rc_background.png" ;
const char *wallImageFile = "_rc_wall.png" ; 
const char *noneImageFile = "_rc_none.png" ;
const char *otherImageFile = "_rc_other.png" ;
const char *errorImageFile = "_rc_error.png" ;





// ------------------------ declaration ------------------------


// �q��function
char *utf8( char *str ) ; // �ѨM�L�k��ܤ��媺�~��
GString *positionToGString( int x, int y ) ; // �Nx�My�զX��" ( x , y ) "�r���^��
GString *numTostr( int num ) ; // �Ʀr��r��

// �樫��function
void walk( GdkEventKey *event ) ; // �V�e���A�������e�n���ˬd�O�_�i�� 
void moveBox( Mt type, int direction, int x, int y ) ; // �N���(x,y)��box�Hdirection����V����
gboolean typeIsRoadOrtTarget( int x, int y ) ; // ( x, y )�����O���a���ΥؼЦa 
gboolean canWalk( int direction, int x, int y ) ; // �i����(x,y)�ܡH

// �a�ϥ�function
void readMap() ; // Ū�J�a���� 
void drawMap() ; // �̾�Ū�J���a����ø�X�C���a��
void restoreMap() ; // �٭�a�Ϩ�e�@�������A 
void recordMap() ; // �O���ثe�a�� 
GtkWidget *setComboBox() ; // �]�mcomboBox�A�i�����󴫦a��
 char *mapStr( int no ) ; // �Hno���w�n�^�ǲĴX�i�a�Ϧr��
void readMapStr( int no ) ; // Ū�J�a�Ϧr�� 

// callback function
gboolean key_callback( GtkWidget *widget, GdkEventKey *event, gpointer data ) ; // �V�U��L�|�ήɤ�����callback function
gboolean combo_changed(GtkComboBox *comboBox, gpointer window ) ; // �]��������a�ϧ@��� 

// �Ϥ���function
void setImage( int type, int x, int y ) ; // �]�m�Ϥ�
GtkWidget *setImageInTable( const gchar *filename, GtkWidget *table, int x, int y )  ; // �N������Ϥ���ܦb�e���]box�^�W 

// ���O��function
void setPositionType( Mt type, int x, int y )  ; // ���s�]�w( x, y )�����O 
Mt getPositionType( int x, int y )  ; // ���o( x, y )�����O 

// �y�{��function
gboolean finishGame() ; // �P�_�O�_�w�g�����o�@���F  
gboolean canUndo() ; // �p��i�_�A�~��undo

// ��l��function
void gameSet() ; // �]�w�C����l��
void mapStateInitialization( Mt map[HEIGHT][WIDTH] ) ; // ��l��map 
void setWorkmanInitialPosition() ; // �]�m�ܮw�u�H���_�l�I
void gMapTypeTempInitialization() ; // �NgMapType����ƥ��ƽƻs��gMapTypeTemp 
void valueInitialization() ; // �Ҧ��ƭ��k�s 




// ------------------------ definition ------------------------


  
char *utf8( char *str )
// �ѨM�L�k��ܤ��媺�~��
{
  return g_locale_to_utf8( str, -1, NULL, NULL, NULL);
}

GString *numToStr( int num )
// �Ʀr��r��
{ 
    GString *numOfStr = g_string_new( "" )  ;
    g_string_printf( numOfStr, "%d", num); // �N�Ʀr�ର�r��s�JnumOfStr 
    return numOfStr ;
}
 
GString *positionToGString( int x, int y )
// �Nx�My�զX���u ( x , y ) �v�r���^��
{ 
    GString *pos = g_string_new( "  ( " ) ; 
    GString *tempStr = g_string_new( "" ) ;
       
    g_string_printf( tempStr, "%d", x ) ;
    g_string_append( pos, tempStr->str ) ;
    g_string_append( pos, " , " ) ;
    g_string_printf( tempStr, "%d", y ) ;
    g_string_append( pos, tempStr->str ) ;
    g_string_append( pos, " )" ) ;
     
    return pos ;
} 

 
void setImage( int type, int x, int y )
// �]�m�Ϥ�  
/*
    workman , // �ܮw�u�H
    box ,     // �n�h���c�l
    boxOpen , // �c�l��b�ؼФW�᪺���A
    target ,  // �c�l�n�������ؼ�
    road , // �i�樫���a�O
    background , // ����H�~���a��
    wall , // ���
    none ,  // �S���F�� 
    other   // ���`�����Ӧ��o�تF��~��......debug��
*/
{ 
    switch( type ) {  
        case workman : 
            gtk_image_set_from_file( GTK_IMAGE( gImage[x][y] ), workmanImageFile ) ; 
            break ;
        case box : 
            gtk_image_set_from_file( GTK_IMAGE( gImage[x][y] ), boxImageFile ) ; 
            break ; 
        case boxOpen : 
            gtk_image_set_from_file( GTK_IMAGE( gImage[x][y] ), boxOpenImageFile ) ; 
            break ; 
        case target : 
            gtk_image_set_from_file( GTK_IMAGE( gImage[x][y] ), targetImageFile ) ; 
            break ; 
        case road : 
            gtk_image_set_from_file( GTK_IMAGE( gImage[x][y] ), roadImageFile ) ; 
            break ; 
        case background : 
            gtk_image_set_from_file( GTK_IMAGE( gImage[x][y] ), backgroundImageFile ) ; 
            break ; 
        case wall : 
            gtk_image_set_from_file( GTK_IMAGE( gImage[x][y] ), wallImageFile ) ; 
            break ; 
        case none : 
            gtk_image_set_from_file( GTK_IMAGE( gImage[x][y] ), noneImageFile ) ; 
            break ; 
        case other : 
            gtk_image_set_from_file( GTK_IMAGE( gImage[x][y] ), otherImageFile ) ; 
            break ; 
        default : 
            gtk_image_set_from_file( GTK_IMAGE( gImage[x][y] ), errorImageFile ) ;
            break ;
    }
    gMapTypeTemp[x][y] = type ;
} 

gboolean typeIsRoadOrtTarget( int x, int y )
// ( x, y )�����O���a���ΥؼЦa 
{
    if ( gMapType[x][y] EQU road || 
         gMapType[x][y] EQU target ) 
        return TRUE ;
    else
        return FALSE ;
}

gboolean canWalk( int direction, int x, int y )
// �i����(x,y)�ܡH
// direction : ��������V
// x, y : �ت��a 
{
    gboolean ans = FALSE ;
    
    if ( typeIsRoadOrtTarget( x, y ) ) { // �ت��a�O�a���Υؼ��I 
        ans = TRUE ;            
    }
    else if ( gMapType[x][y] EQU box || 
              gMapType[x][y] EQU boxOpen ) { // �ت��a�O�c�l�ΥؼФW���c�l  
              
        if ( direction EQU right && typeIsRoadOrtTarget( x, y+1 ) )
            ans = TRUE ;
        else if ( direction EQU left && typeIsRoadOrtTarget( x, y-1 ) )
            ans = TRUE ; 
        else if ( direction EQU up && typeIsRoadOrtTarget( x-1, y ) )
            ans = TRUE ;
        else if ( direction EQU down && typeIsRoadOrtTarget( x+1, y ) )
            ans = TRUE ;
        else
            ans = FALSE ;         
    }
    else
        ans = FALSE ;

    return ans ;
} 

void setPositionType( Mt type, int x, int y ) 
// ���s�]�w( x, y )�����O 
{
    gMapType[x][y] = type ;
}

Mt getPositionType( int x, int y ) 
// ���o( x, y )�����O 
{
    return gMapType[x][y] ;
}

void moveBox( Mt type, int direction, int x, int y )
// �N���(x,y)��box�Hdirection����V����
{  
    gCountOfPushBox ++ ; // ���ʽc�l�����ƥ[�@ 
    
    if ( type EQU box ) 
        setPositionType( road, x, y ) ; // �T�w�n���ʡA�ҥH�����ثe��m�]���a�� 
    else if ( type EQU boxOpen )
        setPositionType( target, x, y ) ; // �]��boxOpen�N�Otarget�W�����Fbox�Abox���ʴN�{�X�쫬�F 
    
    switch( direction ) {
        case right :
            if ( gMapType[x][y+1] EQU target ) {
                setImage( boxOpen, x, y+1 ) ;
                setPositionType( boxOpen, x, y+1 ) ;
            }
            else {
                setImage( box, x, y+1 ) ;
                setPositionType( box, x, y+1 ) ;
            }
            break ;
        case left :
            if ( gMapType[x][y-1] EQU target ) {
                setImage( boxOpen, x, y-1 ) ;
                setPositionType( boxOpen, x, y-1 ) ;
            }
            else {   
                setImage( box, x, y-1 ) ;
                setPositionType( box, x, y-1 ) ;
            } 
            break ;
        case up :
            if ( gMapType[x-1][y] EQU target ) {
                setImage( boxOpen, x-1, y ) ;
                setPositionType( boxOpen, x-1, y ) ;
            }
            else {
                setImage( box, x-1, y ) ;
                setPositionType( box, x-1, y ) ;
            }
            break ;
        case down :
            if ( gMapType[x+1][y] EQU target ) {
                setImage( boxOpen, x+1, y ) ;
                setPositionType( boxOpen, x+1, y ) ;
            }
            else {
                setImage( box, x+1, y ) ;
                setPositionType( box, x+1, y ) ;
            }
            break ;
        default :
            setImage( other, x, y ) ;
            break ;
    }       
}

void walk( GdkEventKey *event )
// �V�e���A�������e�n���ˬd�O�_�i�� 
{
    Dt direction ;
    int x = 0, y = 0 ;
    
    switch( event->keyval ) { // �u�H���V���� 
        case GDK_Up : 
            direction = up ;
            x = gWorkmanX - 1 ;
            y = gWorkmanY ;
            break ; 
        case GDK_Down : 
            direction = down ;
            x = gWorkmanX + 1 ;
            y = gWorkmanY ;
            break ; 
        case GDK_Right : 
            direction = right ;
            x = gWorkmanX ;
            y = gWorkmanY + 1 ;
            break ; 
        case GDK_Left : 
            direction = left ;
            x = gWorkmanX ;
            y = gWorkmanY - 1 ;
            break ; 
        default : 
            x = gWorkmanX ;
            y = gWorkmanY ;
            break ;
    }     
    
    if ( canWalk( direction, x, y ) ) { // �Y�i����(x,y)
    
        setImage( gMapType[gWorkmanX][gWorkmanY], gWorkmanX, gWorkmanY ) ; // �٭�]�w  
        
        if ( gMapType[x][y] EQU box )  
            moveBox( box, direction, x, y ) ; // ���ʫ�Abox -> road 
        else if ( gMapType[x][y] EQU boxOpen ) 
            moveBox( boxOpen, direction, x, y ) ; // ���ʫ�Abox -> target 
        
        setImage( workman, x, y ) ;
        gWorkmanX = x ;
        gWorkmanY = y ;   
    }
}


gboolean key_callback( GtkWidget *widget, GdkEventKey *event, gpointer data )
// �V�U��L�|�ήɤ�����callback function
{
    GtkWidget *label = (GtkWidget *)data;
    GString *pos, *strOfCountOfPushBox, *tempStr ;
    
    guint keyvalue = event->keyval;
    gchar temp[100];
    strcpy( temp, utf8( "���c���ơG" ) );
    
    if( !strcmp( gdk_keyval_name( event->keyval ), "space" ) ) {
        strcat( temp, utf8( "" ) ) ;
        restoreMap() ; // �٭�a�Ϫ��A 
    } 
    else {
        recordMap() ; // �����e�������ثe�a�Ϫ��A
        
        walk( event ) ; // �����D�n�禡 
    }
    strOfCountOfPushBox = numToStr( gCountOfPushBox ) ; // ���o�ثe�y���c�`�ơz���r�� 
    strcat( temp, strOfCountOfPushBox->str ) ;
    strcat( temp, utf8( "�@���ʦ��ơG" ) ) ;
    
    tempStr = numToStr( gUndoNow ) ;
    strcat( temp, tempStr->str ) ;
      
    pos = positionToGString( gWorkmanX, gWorkmanY ) ; // ���o�y�Цr�� 
    // strcat( temp, pos->str ); // ��ܥثe�ܮw�u�H���y�Ц�m 
    
    gtk_label_set_text ( GTK_LABEL( label ), temp );
    
    if ( finishGame() )
        gtk_entry_set_text( GTK_ENTRY( widget ), utf8( "�AĹ�F�I���ߡI�I" ) );
    else
        gtk_entry_set_text( GTK_ENTRY( widget ), utf8( "��space��h�^�e�@�B" ) );
    
    return FALSE;
}

GtkWidget *setImageInTable( const gchar *filename, GtkWidget *table, int x, int y ) 
// �N������Ϥ���ܦb�e���]box�^�W 
{
    GtkWidget *image; // �ŧi��static�A�i�O�s�Ϥ���ƨ�U���ϥ� 
    
    image = gtk_image_new_from_file( filename );
    
    gtk_table_attach_defaults( // �Nlabel��Jtable 
                GTK_TABLE(table), image, x, x + 1, y, y + 1);
    
    
    gtk_widget_show(image);
    
    return image ;
}

char *mapStr( int no )
// �Hno���w�n�^�ǲĴX�i�a�Ϧr��
{
    char *map1 = "\
�ġġ������ġġ�\n\
�ġġ��I��\n\
�ġġ��@��������\n\
�������ϡ@�ϢI��\n\
���I�@�ϡH������\n\
���������ϡ�\n\
�ġġġ��I��\n\
�ġġġ�����\n" ;


    char *map2 = "\
����������\n\
���H�@�@��\n\
���@�ϡϡ��@������\n\
���@�ϡ@���@���I��\n\
�������@�������I��\n\
�ġ����@�@�@�@�I��\n\
�ġ��@�@�@���@�@��\n\
�ġ��@�@�@��������\n\
�ġ���������\n" ;

    char *map3 = "\
�ġ�������������\n\
�ġ��@�@�@�@�@������\n\
�����ϡ������@�@�@��\n\
���@�H�@�ϡ@�@�ϡ@��\n\
���@�I�I���@�ϡ@����\n\
�����I�I���@�@�@��\n\
�@����������������\n" ;

    char *map4 = "\
�ġ�������\n\
�����@�@��\n\
���H�ϡ@��\n\
�����ϡ@����\n\
�����@�ϡ@��\n\
���I�ϡ@�@��\n\
���I�I�עI��\n\
������������\n" ;

    char *map5 = "\
�ġ���������\n\
�ġ��H�@������\n\
�ġ��@�ϡ@�@��\n\
�������@���@����\n\
���I���@���@�@��\n\
���I�ϡ@�@���@��\n\
���I�@�@�@�ϡ@��\n\
����������������\n" ;

    char *map6 = "\
�ġġġ�������������\n\
���������@�@�@�@�@��\n\
���@�@�@�I�������@��\n\
���@���@���@�@�@�@����\n\
���@���@�ϡ@�ϡ��I�@��\n\
���@���@�@�ס@�@���@��\n\
���@�I���ϡ@�ϡ@���@��\n\
�����@�@�@�@���@���@������\n\
�ġ��@�������I�@�@�@�@�H��\n\
�ġ��@�@�@�@�@�����@�@�@��\n\
�ġ�����������������������\n" ;
    


    if ( no EQU 1 )
        return map1 ;
    else if ( no EQU 2 )
        return map2 ;
    else if ( no EQU 3 )
        return map3 ;
    else if ( no EQU 4 )
        return map4  ;
    else if ( no EQU 5 )
        return map5 ;
    else if ( no EQU 6 )
        return map6 ;
    else
        return map1 ;

} 
 
void readMapStr( int no )
// Ū�J�a�Ϧr�� 
{
    char c, frontC ;
    int x = 0, y = 0 ;
    int i = 0, j = 0 ;
    
    char *ms = mapStr( no ) ; // ���o�a�Ϧr�� 
    
    while( ms[i] ) {
        c = ms[i] ;
        
        if ( c EQU 10 ) {
            x ++ ; // ���� = X���U�[1
            y = 0 ; //���� = y�k�s
        }
        else { 
            frontC = c ;
            c = ms[++i] ;
            if ( c ) {
                if ( frontC EQU -95 && c EQU 72 ) {
                    gMapType[x][y++] = workman ;
                }
                else if ( frontC EQU -95 && c EQU 64 ) {
                    gMapType[x][y++] = road ; 
                }
                else if ( frontC EQU -95 && c EQU -41 ) {
                    gMapType[x][y++] = boxOpen ;
                }
                else if ( frontC EQU -95 && c EQU -49 ) {
                    gMapType[x][y++] = box ;
                }
                else if ( frontC EQU -95 && c EQU -60 ) {
                    gMapType[x][y++] = background ;
                } 
                else if ( frontC EQU -95 && c EQU -81 ) {
                    gMapType[x][y++] = wall ;
                }
                else if ( frontC EQU -94 && c EQU 73 ) { 
                    gMapType[x][y++] = target ;
                }
                else { 
                    gMapType[x][y++] = other ;
                }
            }
        }
        i ++ ;
    } 
} 

void readMap()
// Ū�J�a���� 
{
    char c, frontC ;
    int x = 0, y = 0 ;
    int i = 0, j = 0 ;
    FILE *fp;
    
    fp = fopen( mapFile, "r" ); // Ū�J�a���� 
    
    while( fp && ( c = fgetc( fp ) ) != EOF ) {
        if ( c EQU 10 ) {
            x ++ ; // ���� = X���U�[1
            y = 0 ; //���� = y�k�s
        }
        else { 
            frontC = c ;
            c = fgetc( fp ) ;
            if ( c != EOF ) {
                if ( frontC EQU -95 && c EQU 72 ) {
                    gMapType[x][y++] = workman ;
                }
                else if ( frontC EQU -95 && c EQU 64 ) {
                    gMapType[x][y++] = road ; 
                }
                else if ( frontC EQU -95 && c EQU -41 ) {
                    gMapType[x][y++] = boxOpen ;
                }
                else if ( frontC EQU -95 && c EQU -49 ) {
                    gMapType[x][y++] = box ;
                }
                else if ( frontC EQU -95 && c EQU -60 ) {
                    gMapType[x][y++] = background ;
                } 
                else if ( frontC EQU -95 && c EQU -81 ) {
                    gMapType[x][y++] = wall ;
                }
                else if ( frontC EQU -94 && c EQU 73 ) { 
                    gMapType[x][y++] = target ;
                }
                else { 
                    gMapType[x][y++] = other ;
                }
            }
        }
    } 
    
    if ( fp ) 
        fclose( fp ) ; // �����a���� 
}

void drawMap()
// �̾�Ū�J���a����ø�X�C���a��
{
    int i = 0, j = 0 ;
    for ( i = 0 ; i < HEIGHT ; i ++ )
        for ( j = 0 ; j < WIDTH ; j ++ )
            setImage( gMapType[i][j], i, j ) ;

} 

void gMapTypeTempInitialization()
// �NgMapType����ƥ��ƽƻs��gMapTypeTemp 
{
    int i = 0, j = 0 ;
    for ( i = 0 ; i < HEIGHT ; i ++ ) 
        for ( j = 0 ; j < WIDTH ; j ++ ) 
            gMapTypeTemp[i][j] = gMapType[i][j] ;

}

void recordMap()
// �O���ثe�a�� 
{
    gUndoNowInArrary = ( ++ gUndoNow ) % COUNT_OF_RECORD ; 
    
    int i = 0, j = 0 ;
    for ( i = 0 ; i < HEIGHT ; i ++ ) {
        for ( j = 0 ; j < WIDTH ; j ++ ) {
            gUndoData[gUndoNowInArrary].mapType[i][j] = gMapTypeTemp[i][j] ;   
        }
    }
    
    gUndoData[gUndoNowInArrary].workmanX = gWorkmanX ; // ����workman�Ҧb 
    gUndoData[gUndoNowInArrary].workmanY = gWorkmanY ;
    
    gUndoData[gUndoNowInArrary].countOfPushBox = gCountOfPushBox ; // ������U�����c���� 
    
    gUndoMaxValue = gUndoNow ; // �̰��F�쪺�ƭȡA�Ω�p��i�^�������I 
    
}

gboolean canUndo()
// �p��i�_�A�~��undo
{
    gboolean undo = FALSE ;
    if ( gUndoNow > 0 ) {
        if ( gUndoMaxValue > 0 && gUndoMaxValue < COUNT_OF_RECORD ) 
            undo = TRUE ; // �b1 ~ ������-1�����A�۵M�S�����ƪ����D�o�� 
        else if ( gUndoNow > ( gUndoMaxValue - COUNT_OF_RECORD ) )
            undo = TRUE ; // undo�S���W�X�d�� 
        else
            undo = FALSE ;
    }
    else
        undo = FALSE ;
        
    return undo ; 
} 

void restoreMap()
// �٭�a�Ϩ�e�@�������A 
{
    int i = 0, j = 0 ;
    
    if ( canUndo() ) {
        for ( i = 0 ; i < HEIGHT ; i ++ ) {
            for ( j = 0 ; j < WIDTH ; j ++ ) {
                // �]����ڤW�A�ä��|��workman�g�J�a�����O���A�u�|��ܥX�ӦӤw�C 
                if ( !gUndoData[gUndoNowInArrary].mapType[i][j] EQU workman ) 
                    setPositionType( gUndoData[gUndoNowInArrary].mapType[i][j], i, j ) ;
                setImage( gUndoData[gUndoNowInArrary].mapType[i][j], i, j ) ; 
            }
        }
        gWorkmanX = gUndoData[gUndoNowInArrary].workmanX ;
        gWorkmanY = gUndoData[gUndoNowInArrary].workmanY ;
        
        gCountOfPushBox = gUndoData[gUndoNowInArrary].countOfPushBox ; // �٭�e�������c���� 
        
        // �G���o�w���@�k�A�]�� setImage���̫�|�g�JgMapTypeTemp�A�ҥH�u���o�Ӯ��X�ӥΡC 
        gtk_image_set_from_file( GTK_IMAGE( gImage[gWorkmanX][gWorkmanY] ), workmanImageFile ) ;
        
        gUndoNowInArrary = ( -- gUndoNow ) % COUNT_OF_RECORD ;
    }
}
 

void mapStateInitialization( Mt map[HEIGHT][WIDTH] )
// ��l��map 
{
    int i = 0, j = 0 ;
    for ( i = 0 ; i < HEIGHT ; i ++ )
        for ( j = 0 ; j < WIDTH ; j ++ )
            map[i][j] = none ;

}

void setWorkmanInitialPosition()
// �]�m�ܮw�u�H���_�l�I
{
    int i = 0, j = 0 ;
    gboolean over = FALSE ;
    for ( i = 0 ; i < HEIGHT && !over ; i ++ )
        for ( j = 0 ; j < WIDTH && !over ; j ++ )
            if ( gMapType[i][j] EQU workman ) {
                gWorkmanX = i ;
                gWorkmanY = j ;
                over = TRUE ;
            }
    
    gMapType[gWorkmanX][gWorkmanY] = road ; // ���}��l�I�����i���������@ 
} 

void valueInitialization()
// �Ҧ��ƭ��k�s 
{
    gCountOfPushBox = 0 ; // ���ʽc�l������ 
    gUndoNow = 0 ; // �ثe�O���X���a�ϤF 
    gUndoNowInArrary = 0 ; // ���arrary����gUndoNow���� 
    gUndoMaxValue = 0 ; // gUndoNow�̰��F�쪺�ȡA�u�|�[�A���|��C 
}

void gameSet( int mapNo )
// �]�w�C����l��
{
    
    //gtk_image_set_from_file( GTK_IMAGE( gImage[4][1] ), boxOpenImageFile ) ; 
    
    valueInitialization() ; // �Ҧ��ƭ��k�s 
    
    mapStateInitialization( gMapType ) ; // ��l��map 
    
    readMapStr( mapNo ) ; // Ū�J�a���� 
    drawMap() ; // �̾�Ū�J���a����ø�X�C���a��
    
    setWorkmanInitialPosition() ; // // �]�m�ܮw�u�H���_�l�I
    
    gMapTypeTempInitialization() ; // �NgMapType����ƥ��ƽƻs��gMapTypeTemp�Aundo�� 
    
} 

gboolean finishGame()
// �P�_�O�_�w�g�����o�@���F 
{
    gboolean finish = TRUE ;
    
    int i = 0, j = 0 ;
    
    for ( i = 0 ; i < HEIGHT ; i ++ ) 
        for ( j = 0 ; j < WIDTH ; j ++ ) 
            if ( gMapType[i][j] EQU box || 
                 gMapType[i][j] EQU target )
                finish = FALSE ; // �N���٦��c�l�S�i��ؼи̭��h 
    
    return finish ;
}


GtkWidget *setInitialTable()
// �]�mtable��l��
{
    
    GtkWidget *label ;
    GtkWidget *table = gtk_table_new( 2, 10, TRUE ) ; 
    GString *tempStr = g_string_new( "" ) ;
    
    int i = 0, j = 0 ;
    
    for ( i = 0 ; i < HEIGHT ; i ++ ) {
        for ( j = 0 ; j < WIDTH ; j ++ ) { 
            g_string_printf( tempStr, "%d", i*10+j ) ;
            label = gtk_label_new( "" ) ; 
            gLabel[i][j] = label ;
            gtk_table_attach_defaults( // �Nlabel��Jtable 
                GTK_TABLE( table ), label, j, j + 1, i, i + 1) ; 
            gImage[i][j] = setImageInTable( backgroundImageFile, table, j, i ) ; // ��l�� 
        }
    }
    
    return table ;
}


gboolean combo_changed( GtkComboBox *comboBox, gpointer entry ) 
// �]��������a�ϧ@��� 
{
    gchar *active = gtk_combo_box_get_active_text( comboBox ) ;
    int mapNo =  atoi( active ) ; // �N�ҿ諸�a�Ϭ����_�� 
    
    // �]�����UTab��~�����L�����v���ٵ�entry�A�]�~�i����key_callback 
    gtk_entry_set_text( GTK_ENTRY( entry ), utf8( "�Ы�Tab��}�l�C��" ) );
    
    gameSet( mapNo ) ; // ���}�a�� 
}

GtkWidget *setComboBox()
// �]�mcomboBox�A�i�����󴫦a�� 
{
    GtkWidget *comboBox = gtk_combo_box_new_text();
    gtk_combo_box_append_text(GTK_COMBO_BOX(comboBox), utf8( "�󴫦a��" ));
    gtk_combo_box_append_text(GTK_COMBO_BOX(comboBox), "1");
    gtk_combo_box_append_text(GTK_COMBO_BOX(comboBox), "2");
    gtk_combo_box_append_text(GTK_COMBO_BOX(comboBox), "3");
    gtk_combo_box_append_text(GTK_COMBO_BOX(comboBox), "4");
    gtk_combo_box_append_text(GTK_COMBO_BOX(comboBox), "5");
    gtk_combo_box_append_text(GTK_COMBO_BOX(comboBox), "6");
    gtk_combo_box_set_active(GTK_COMBO_BOX(comboBox), 0);
    
    return comboBox ;
}

int main(int argc, char *argv[])
{
    GtkWidget *window ;
    GtkWidget *table ;
    GtkWidget *label , *label2;
    GtkWidget *comboBox ;
    GtkWidget *vbox, *hbox ;
    GtkWidget *entry ;
    GtkWidget *button, *button2 ;
    GtkWidget *gtkFrame1 ;

    gtk_init( &argc, &argv ) ;
    window = gtk_window_new( GTK_WINDOW_TOPLEVEL ) ;
    gtk_window_set_resizable( GTK_WINDOW( window ), FALSE ) ; 
    gtk_window_set_title( GTK_WINDOW( window ), utf8( "�ܮw�f" ) );
    gtk_container_set_border_width( GTK_CONTAINER( window ), 5 ) ;

    
    table = setInitialTable() ; // �Ntable��l�� 

     
    // �L�Ϊ��Y�ơ]��P�椧���`�O���Ż�......�^ 
    gtk_table_set_col_spacings( GTK_TABLE( table ), 0 ) ; 
    gtk_table_set_row_spacings( GTK_TABLE( table ), 0 ) ;  
    
    gtkFrame1 = gtk_frame_new( utf8( "" ) );
    gtk_container_add( GTK_CONTAINER( gtkFrame1 ), table );
    
    vbox = gtk_vbox_new( FALSE, 0 ) ;
    hbox = gtk_hbox_new( FALSE, 0 ) ;
    gtk_container_set_border_width( GTK_CONTAINER( hbox ), 0 ) ;
    
    // �]�mhbox 
    label = gtk_label_new ( utf8( "���c���ơG0�@�@���ʦ��ơG" ) ) ;
    gtk_box_pack_start( GTK_BOX( hbox ), label, FALSE, FALSE, 10 ) ;
    
    entry = gtk_entry_new() ;    
    gtk_entry_set_editable( GTK_ENTRY( entry ), FALSE ) ;  
    gtk_box_pack_start( GTK_BOX( hbox ), entry, FALSE, FALSE, 10 ) ;
    
    button = gtk_button_new_with_label ( "close" );
    // gtk_box_pack_start( GTK_BOX( hbox ), button, FALSE, FALSE, 10 ) ;
    
    label2 = gtk_label_new ( utf8( "�@�a�ϡG" ) ) ;
    gtk_box_pack_start( GTK_BOX( hbox ), label2, FALSE, FALSE, 0 ) ;
    
    comboBox = setComboBox() ; // �]�mcomboBox�A�i�����󴫦a�� 
    gtk_box_pack_start( GTK_BOX( hbox ), comboBox, FALSE, FALSE, 0 ) ;
    
     
    // �]�mvbox 
    gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 ) ;
    gtk_box_pack_start( GTK_BOX( vbox ), gtkFrame1, FALSE, FALSE, 0 ) ;
    gtk_container_add ( GTK_CONTAINER( window ), vbox ) ;
    
    
    gameSet( 1 ) ; // �]�w�C����l�ȡA�w�]�}�Ĥ@�i�a�ϡC 
 

    gtk_widget_show_all( window ) ;
    
    // g_signal_connect( button, "clicked", G_CALLBACK( gtk_main_quit ), NULL ) ;
    g_signal_connect( window, "destroy", G_CALLBACK( gtk_main_quit ), NULL ) ;
    g_signal_connect( entry, "key-press-event", G_CALLBACK( key_callback ), label ) ;
    g_signal_connect(GTK_OBJECT(comboBox), "changed", G_CALLBACK(combo_changed), entry );
    
    gtk_main() ;
    return 0 ;
}
 
