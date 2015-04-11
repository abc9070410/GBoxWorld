/*
GTK實作的簡易倉庫番視窗遊戲。
遊戲中會計算推箱次數和移動次數並顯示在上方label文字中。
以鍵盤的方向鍵控制遊戲中的倉庫工人，進行搬運工作。 
按space鍵可退回前一步，預設為最多退一百步。
以32x32的多張圖檔作為地圖元件，繪製出整張地圖。 
可讀取地圖檔，或使用內建地圖， 內建地圖有六張，可遊戲中直接更換地圖。 
多數訊息會顯示在上方的entry輸入框。 

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
#define WIDTH 17  // 寬 
#define HEIGHT 12 // 長 
#define COUNT_OF_RECORD 100 // 可回溯幾步   

 
/*
 1. 讀取地圖檔 ok
 2. 限定遊走範圍 ok
   2.1. 走向 road or target -> OK
        走向 box -> 需判斷box同個方向是不是為road or target，若是 -> OK 
 3. 設定推動箱子的條件 ok
 4. 制定遊戲完成的條件 ok
 5. 可undo很多步(按鈕+快捷鍵) ok
    5.1. undo時，同時還原推動的箱子次數 ok
    5.2. 從固定次數設成可循環。否則超過次數就會error ok
 6. 紀錄推動箱子的次數 ok
 7. 內建地圖檔 ok
 8. 完成後可直接換下張地圖進行遊戲 ok 
 9. 設計AI 
*/ 


enum directionType { // 四種方向：上、下、左、右 
    right ,
    left ,
    up ,
    down
} ;

typedef enum directionType Dt ;


enum mapType {
    workman , // 倉庫工人
    box ,     // 要搬的箱子
    boxOpen , // 箱子放在目標上後的狀態
    target ,  // 箱子要移往的目標
    road , // 可行走的地板
    background , // 牆壁以外的地面
    wall , // 牆壁
    none ,  // 沒有東西
    other   // 正常不應該有這種東西才對......debug用
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
Mt gMapTypeTemp[HEIGHT][WIDTH] ; // 還原用 
Ud gUndoData[COUNT_OF_RECORD] ; // 還原用 

  
int gWorkmanX = 5 ;  // 預設倉庫工人的位置 
int gWorkmanY = 5 ;  

int gCountOfPushBox = 0 ; // 推動箱子的次數 
int gUndoNow = 0 ; // 目前記錄幾次地圖了 
int gUndoNowInArrary = 0 ; // 實際arrary中的gUndoNow次序 
int gUndoMaxValue = 0 ; // gUndoNow最高達到的值，只會加，不會減。 
  
// 預設的地圖檔  
const char *mapFile = "_map5.txt" ;  

// 有用到的幾個圖片檔
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


// 通用function
char *utf8( char *str ) ; // 解決無法顯示中文的窘境
GString *positionToGString( int x, int y ) ; // 將x和y組合成" ( x , y ) "字串後回傳
GString *numTostr( int num ) ; // 數字轉字串

// 行走用function
void walk( GdkEventKey *event ) ; // 向前走，但走之前要先檢查是否可走 
void moveBox( Mt type, int direction, int x, int y ) ; // 將位於(x,y)的box以direction的方向移動
gboolean typeIsRoadOrtTarget( int x, int y ) ; // ( x, y )的類別為地面或目標地 
gboolean canWalk( int direction, int x, int y ) ; // 可走到(x,y)嗎？

// 地圖用function
void readMap() ; // 讀入地圖檔 
void drawMap() ; // 依據讀入的地圖檔繪出遊戲地圖
void restoreMap() ; // 還原地圖到前一次的狀態 
void recordMap() ; // 記錄目前地圖 
GtkWidget *setComboBox() ; // 設置comboBox，可直接更換地圖
 char *mapStr( int no ) ; // 以no指定要回傳第幾張地圖字串
void readMapStr( int no ) ; // 讀入地圖字串 

// callback function
gboolean key_callback( GtkWidget *widget, GdkEventKey *event, gpointer data ) ; // 敲下鍵盤會及時反應的callback function
gboolean combo_changed(GtkComboBox *comboBox, gpointer window ) ; // 因應選取的地圖作更動 

// 圖片用function
void setImage( int type, int x, int y ) ; // 設置圖片
GtkWidget *setImageInTable( const gchar *filename, GtkWidget *table, int x, int y )  ; // 將選取的圖片顯示在畫面（box）上 

// 類別用function
void setPositionType( Mt type, int x, int y )  ; // 重新設定( x, y )的類別 
Mt getPositionType( int x, int y )  ; // 取得( x, y )的類別 

// 流程用function
gboolean finishGame() ; // 判斷是否已經完成這一局了  
gboolean canUndo() ; // 計算可否再繼續undo

// 初始化function
void gameSet() ; // 設定遊戲初始值
void mapStateInitialization( Mt map[HEIGHT][WIDTH] ) ; // 初始化map 
void setWorkmanInitialPosition() ; // 設置倉庫工人的起始點
void gMapTypeTempInitialization() ; // 將gMapType的資料全數複製到gMapTypeTemp 
void valueInitialization() ; // 所有數值歸零 




// ------------------------ definition ------------------------


  
char *utf8( char *str )
// 解決無法顯示中文的窘境
{
  return g_locale_to_utf8( str, -1, NULL, NULL, NULL);
}

GString *numToStr( int num )
// 數字轉字串
{ 
    GString *numOfStr = g_string_new( "" )  ;
    g_string_printf( numOfStr, "%d", num); // 將數字轉為字串存入numOfStr 
    return numOfStr ;
}
 
GString *positionToGString( int x, int y )
// 將x和y組合成「 ( x , y ) 」字串後回傳
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
// 設置圖片  
/*
    workman , // 倉庫工人
    box ,     // 要搬的箱子
    boxOpen , // 箱子放在目標上後的狀態
    target ,  // 箱子要移往的目標
    road , // 可行走的地板
    background , // 牆壁以外的地面
    wall , // 牆壁
    none ,  // 沒有東西 
    other   // 正常不應該有這種東西才對......debug用
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
// ( x, y )的類別為地面或目標地 
{
    if ( gMapType[x][y] EQU road || 
         gMapType[x][y] EQU target ) 
        return TRUE ;
    else
        return FALSE ;
}

gboolean canWalk( int direction, int x, int y )
// 可走到(x,y)嗎？
// direction : 走路的方向
// x, y : 目的地 
{
    gboolean ans = FALSE ;
    
    if ( typeIsRoadOrtTarget( x, y ) ) { // 目的地是地面或目標點 
        ans = TRUE ;            
    }
    else if ( gMapType[x][y] EQU box || 
              gMapType[x][y] EQU boxOpen ) { // 目的地是箱子或目標上的箱子  
              
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
// 重新設定( x, y )的類別 
{
    gMapType[x][y] = type ;
}

Mt getPositionType( int x, int y ) 
// 取得( x, y )的類別 
{
    return gMapType[x][y] ;
}

void moveBox( Mt type, int direction, int x, int y )
// 將位於(x,y)的box以direction的方向移動
{  
    gCountOfPushBox ++ ; // 推動箱子的次數加一 
    
    if ( type EQU box ) 
        setPositionType( road, x, y ) ; // 確定要移動，所以先行把目前位置設為地表 
    else if ( type EQU boxOpen )
        setPositionType( target, x, y ) ; // 因為boxOpen就是target上面壓了box，box移動就現出原型了 
    
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
// 向前走，但走之前要先檢查是否可走 
{
    Dt direction ;
    int x = 0, y = 0 ;
    
    switch( event->keyval ) { // 工人走向哪邊 
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
    
    if ( canWalk( direction, x, y ) ) { // 若可走到(x,y)
    
        setImage( gMapType[gWorkmanX][gWorkmanY], gWorkmanX, gWorkmanY ) ; // 還原設定  
        
        if ( gMapType[x][y] EQU box )  
            moveBox( box, direction, x, y ) ; // 移動後，box -> road 
        else if ( gMapType[x][y] EQU boxOpen ) 
            moveBox( boxOpen, direction, x, y ) ; // 移動後，box -> target 
        
        setImage( workman, x, y ) ;
        gWorkmanX = x ;
        gWorkmanY = y ;   
    }
}


gboolean key_callback( GtkWidget *widget, GdkEventKey *event, gpointer data )
// 敲下鍵盤會及時反應的callback function
{
    GtkWidget *label = (GtkWidget *)data;
    GString *pos, *strOfCountOfPushBox, *tempStr ;
    
    guint keyvalue = event->keyval;
    gchar temp[100];
    strcpy( temp, utf8( "推箱次數：" ) );
    
    if( !strcmp( gdk_keyval_name( event->keyval ), "space" ) ) {
        strcat( temp, utf8( "" ) ) ;
        restoreMap() ; // 還原地圖狀態 
    } 
    else {
        recordMap() ; // 走之前先紀錄目前地圖狀態
        
        walk( event ) ; // 走路主要函式 
    }
    strOfCountOfPushBox = numToStr( gCountOfPushBox ) ; // 取得目前『推箱總數』的字串 
    strcat( temp, strOfCountOfPushBox->str ) ;
    strcat( temp, utf8( "　移動次數：" ) ) ;
    
    tempStr = numToStr( gUndoNow ) ;
    strcat( temp, tempStr->str ) ;
      
    pos = positionToGString( gWorkmanX, gWorkmanY ) ; // 取得座標字串 
    // strcat( temp, pos->str ); // 顯示目前倉庫工人的座標位置 
    
    gtk_label_set_text ( GTK_LABEL( label ), temp );
    
    if ( finishGame() )
        gtk_entry_set_text( GTK_ENTRY( widget ), utf8( "你贏了！恭喜！！" ) );
    else
        gtk_entry_set_text( GTK_ENTRY( widget ), utf8( "按space鍵退回前一步" ) );
    
    return FALSE;
}

GtkWidget *setImageInTable( const gchar *filename, GtkWidget *table, int x, int y ) 
// 將選取的圖片顯示在畫面（box）上 
{
    GtkWidget *image; // 宣告成static，可保存圖片資料到下次使用 
    
    image = gtk_image_new_from_file( filename );
    
    gtk_table_attach_defaults( // 將label放入table 
                GTK_TABLE(table), image, x, x + 1, y, y + 1);
    
    
    gtk_widget_show(image);
    
    return image ;
}

char *mapStr( int no )
// 以no指定要回傳第幾張地圖字串
{
    char *map1 = "\
＿＿＊＊＊＿＿＿\n\
＿＿＊＠＊\n\
＿＿＊　＊＊＊＊\n\
＊＊＊＋　＋＠＊\n\
＊＠　＋？＊＊＊\n\
＊＊＊＊＋＊\n\
＿＿＿＊＠＊\n\
＿＿＿＊＊＊\n" ;


    char *map2 = "\
＊＊＊＊＊\n\
＊？　　＊\n\
＊　＋＋＊　＊＊＊\n\
＊　＋　＊　＊＠＊\n\
＊＊＊　＊＊＊＠＊\n\
＿＊＊　　　　＠＊\n\
＿＊　　　＊　　＊\n\
＿＊　　　＊＊＊＊\n\
＿＊＊＊＊＊\n" ;

    char *map3 = "\
＿＊＊＊＊＊＊＊\n\
＿＊　　　　　＊＊＊\n\
＊＊＋＊＊＊　　　＊\n\
＊　？　＋　　＋　＊\n\
＊　＠＠＊　＋　＊＊\n\
＊＊＠＠＊　　　＊\n\
　＊＊＊＊＊＊＊＊\n" ;

    char *map4 = "\
＿＊＊＊＊\n\
＊＊　　＊\n\
＊？＋　＊\n\
＊＊＋　＊＊\n\
＊＊　＋　＊\n\
＊＠＋　　＊\n\
＊＠＠＝＠＊\n\
＊＊＊＊＊＊\n" ;

    char *map5 = "\
＿＊＊＊＊＊\n\
＿＊？　＊＊＊\n\
＿＊　＋　　＊\n\
＊＊＊　＊　＊＊\n\
＊＠＊　＊　　＊\n\
＊＠＋　　＊　＊\n\
＊＠　　　＋　＊\n\
＊＊＊＊＊＊＊＊\n" ;

    char *map6 = "\
＿＿＿＊＊＊＊＊＊＊\n\
＊＊＊＊　　　　　＊\n\
＊　　　＠＊＊＊　＊\n\
＊　＊　＊　　　　＊＊\n\
＊　＊　＋　＋＊＠　＊\n\
＊　＊　　＝　　＊　＊\n\
＊　＠＊＋　＋　＊　＊\n\
＊＊　　　　＊　＊　＊＊＊\n\
＿＊　＊＊＊＠　　　　？＊\n\
＿＊　　　　　＊＊　　　＊\n\
＿＊＊＊＊＊＊＊＊＊＊＊＊\n" ;
    


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
// 讀入地圖字串 
{
    char c, frontC ;
    int x = 0, y = 0 ;
    int i = 0, j = 0 ;
    
    char *ms = mapStr( no ) ; // 取得地圖字串 
    
    while( ms[i] ) {
        c = ms[i] ;
        
        if ( c EQU 10 ) {
            x ++ ; // 換行 = X往下加1
            y = 0 ; //換行 = y歸零
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
// 讀入地圖檔 
{
    char c, frontC ;
    int x = 0, y = 0 ;
    int i = 0, j = 0 ;
    FILE *fp;
    
    fp = fopen( mapFile, "r" ); // 讀入地圖檔 
    
    while( fp && ( c = fgetc( fp ) ) != EOF ) {
        if ( c EQU 10 ) {
            x ++ ; // 換行 = X往下加1
            y = 0 ; //換行 = y歸零
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
        fclose( fp ) ; // 關閉地圖檔 
}

void drawMap()
// 依據讀入的地圖檔繪出遊戲地圖
{
    int i = 0, j = 0 ;
    for ( i = 0 ; i < HEIGHT ; i ++ )
        for ( j = 0 ; j < WIDTH ; j ++ )
            setImage( gMapType[i][j], i, j ) ;

} 

void gMapTypeTempInitialization()
// 將gMapType的資料全數複製到gMapTypeTemp 
{
    int i = 0, j = 0 ;
    for ( i = 0 ; i < HEIGHT ; i ++ ) 
        for ( j = 0 ; j < WIDTH ; j ++ ) 
            gMapTypeTemp[i][j] = gMapType[i][j] ;

}

void recordMap()
// 記錄目前地圖 
{
    gUndoNowInArrary = ( ++ gUndoNow ) % COUNT_OF_RECORD ; 
    
    int i = 0, j = 0 ;
    for ( i = 0 ; i < HEIGHT ; i ++ ) {
        for ( j = 0 ; j < WIDTH ; j ++ ) {
            gUndoData[gUndoNowInArrary].mapType[i][j] = gMapTypeTemp[i][j] ;   
        }
    }
    
    gUndoData[gUndoNowInArrary].workmanX = gWorkmanX ; // 紀錄workman所在 
    gUndoData[gUndoNowInArrary].workmanY = gWorkmanY ;
    
    gUndoData[gUndoNowInArrary].countOfPushBox = gCountOfPushBox ; // 紀錄當下的推箱次數 
    
    gUndoMaxValue = gUndoNow ; // 最高達到的數值，用於計算可回溯的終點 
    
}

gboolean canUndo()
// 計算可否再繼續undo
{
    gboolean undo = FALSE ;
    if ( gUndoNow > 0 ) {
        if ( gUndoMaxValue > 0 && gUndoMaxValue < COUNT_OF_RECORD ) 
            undo = TRUE ; // 在1 ~ 極限值-1之間，自然沒有重複的問題發生 
        else if ( gUndoNow > ( gUndoMaxValue - COUNT_OF_RECORD ) )
            undo = TRUE ; // undo沒有超出範圍 
        else
            undo = FALSE ;
    }
    else
        undo = FALSE ;
        
    return undo ; 
} 

void restoreMap()
// 還原地圖到前一次的狀態 
{
    int i = 0, j = 0 ;
    
    if ( canUndo() ) {
        for ( i = 0 ; i < HEIGHT ; i ++ ) {
            for ( j = 0 ; j < WIDTH ; j ++ ) {
                // 因為實際上，並不會把workman寫入地圖類別中，只會顯示出來而已。 
                if ( !gUndoData[gUndoNowInArrary].mapType[i][j] EQU workman ) 
                    setPositionType( gUndoData[gUndoNowInArrary].mapType[i][j], i, j ) ;
                setImage( gUndoData[gUndoNowInArrary].mapType[i][j], i, j ) ; 
            }
        }
        gWorkmanX = gUndoData[gUndoNowInArrary].workmanX ;
        gWorkmanY = gUndoData[gUndoNowInArrary].workmanY ;
        
        gCountOfPushBox = gUndoData[gUndoNowInArrary].countOfPushBox ; // 還原前次的推箱次數 
        
        // 逼不得已的作法，因為 setImage中最後會寫入gMapTypeTemp，所以只能把這個拿出來用。 
        gtk_image_set_from_file( GTK_IMAGE( gImage[gWorkmanX][gWorkmanY] ), workmanImageFile ) ;
        
        gUndoNowInArrary = ( -- gUndoNow ) % COUNT_OF_RECORD ;
    }
}
 

void mapStateInitialization( Mt map[HEIGHT][WIDTH] )
// 初始化map 
{
    int i = 0, j = 0 ;
    for ( i = 0 ; i < HEIGHT ; i ++ )
        for ( j = 0 ; j < WIDTH ; j ++ )
            map[i][j] = none ;

}

void setWorkmanInitialPosition()
// 設置倉庫工人的起始點
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
    
    gMapType[gWorkmanX][gWorkmanY] = road ; // 離開初始點必為可走之路面　 
} 

void valueInitialization()
// 所有數值歸零 
{
    gCountOfPushBox = 0 ; // 推動箱子的次數 
    gUndoNow = 0 ; // 目前記錄幾次地圖了 
    gUndoNowInArrary = 0 ; // 實際arrary中的gUndoNow次序 
    gUndoMaxValue = 0 ; // gUndoNow最高達到的值，只會加，不會減。 
}

void gameSet( int mapNo )
// 設定遊戲初始值
{
    
    //gtk_image_set_from_file( GTK_IMAGE( gImage[4][1] ), boxOpenImageFile ) ; 
    
    valueInitialization() ; // 所有數值歸零 
    
    mapStateInitialization( gMapType ) ; // 初始化map 
    
    readMapStr( mapNo ) ; // 讀入地圖檔 
    drawMap() ; // 依據讀入的地圖檔繪出遊戲地圖
    
    setWorkmanInitialPosition() ; // // 設置倉庫工人的起始點
    
    gMapTypeTempInitialization() ; // 將gMapType的資料全數複製到gMapTypeTemp，undo用 
    
} 

gboolean finishGame()
// 判斷是否已經完成這一局了 
{
    gboolean finish = TRUE ;
    
    int i = 0, j = 0 ;
    
    for ( i = 0 ; i < HEIGHT ; i ++ ) 
        for ( j = 0 ; j < WIDTH ; j ++ ) 
            if ( gMapType[i][j] EQU box || 
                 gMapType[i][j] EQU target )
                finish = FALSE ; // 代表還有箱子沒進到目標裡面去 
    
    return finish ;
}


GtkWidget *setInitialTable()
// 設置table初始化
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
            gtk_table_attach_defaults( // 將label放入table 
                GTK_TABLE( table ), label, j, j + 1, i, i + 1) ; 
            gImage[i][j] = setImageInTable( backgroundImageFile, table, j, i ) ; // 初始化 
        }
    }
    
    return table ;
}


gboolean combo_changed( GtkComboBox *comboBox, gpointer entry ) 
// 因應選取的地圖作更動 
{
    gchar *active = gtk_combo_box_get_active_text( comboBox ) ;
    int mapNo =  atoi( active ) ; // 將所選的地圖紀錄起來 
    
    // 因為按下Tab鍵才能把鍵盤控制權交還給entry，也才可對應key_callback 
    gtk_entry_set_text( GTK_ENTRY( entry ), utf8( "請按Tab鍵開始遊戲" ) );
    
    gameSet( mapNo ) ; // 重開地圖 
}

GtkWidget *setComboBox()
// 設置comboBox，可直接更換地圖 
{
    GtkWidget *comboBox = gtk_combo_box_new_text();
    gtk_combo_box_append_text(GTK_COMBO_BOX(comboBox), utf8( "更換地圖" ));
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
    gtk_window_set_title( GTK_WINDOW( window ), utf8( "倉庫番" ) );
    gtk_container_set_border_width( GTK_CONTAINER( window ), 5 ) ;

    
    table = setInitialTable() ; // 將table初始化 

     
    // 無用的縮排（行與行之間總是有空隙......） 
    gtk_table_set_col_spacings( GTK_TABLE( table ), 0 ) ; 
    gtk_table_set_row_spacings( GTK_TABLE( table ), 0 ) ;  
    
    gtkFrame1 = gtk_frame_new( utf8( "" ) );
    gtk_container_add( GTK_CONTAINER( gtkFrame1 ), table );
    
    vbox = gtk_vbox_new( FALSE, 0 ) ;
    hbox = gtk_hbox_new( FALSE, 0 ) ;
    gtk_container_set_border_width( GTK_CONTAINER( hbox ), 0 ) ;
    
    // 設置hbox 
    label = gtk_label_new ( utf8( "推箱次數：0　　移動次數：" ) ) ;
    gtk_box_pack_start( GTK_BOX( hbox ), label, FALSE, FALSE, 10 ) ;
    
    entry = gtk_entry_new() ;    
    gtk_entry_set_editable( GTK_ENTRY( entry ), FALSE ) ;  
    gtk_box_pack_start( GTK_BOX( hbox ), entry, FALSE, FALSE, 10 ) ;
    
    button = gtk_button_new_with_label ( "close" );
    // gtk_box_pack_start( GTK_BOX( hbox ), button, FALSE, FALSE, 10 ) ;
    
    label2 = gtk_label_new ( utf8( "　地圖：" ) ) ;
    gtk_box_pack_start( GTK_BOX( hbox ), label2, FALSE, FALSE, 0 ) ;
    
    comboBox = setComboBox() ; // 設置comboBox，可直接更換地圖 
    gtk_box_pack_start( GTK_BOX( hbox ), comboBox, FALSE, FALSE, 0 ) ;
    
     
    // 設置vbox 
    gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 ) ;
    gtk_box_pack_start( GTK_BOX( vbox ), gtkFrame1, FALSE, FALSE, 0 ) ;
    gtk_container_add ( GTK_CONTAINER( window ), vbox ) ;
    
    
    gameSet( 1 ) ; // 設定遊戲初始值，預設開第一張地圖。 
 

    gtk_widget_show_all( window ) ;
    
    // g_signal_connect( button, "clicked", G_CALLBACK( gtk_main_quit ), NULL ) ;
    g_signal_connect( window, "destroy", G_CALLBACK( gtk_main_quit ), NULL ) ;
    g_signal_connect( entry, "key-press-event", G_CALLBACK( key_callback ), label ) ;
    g_signal_connect(GTK_OBJECT(comboBox), "changed", G_CALLBACK(combo_changed), entry );
    
    gtk_main() ;
    return 0 ;
}
 
