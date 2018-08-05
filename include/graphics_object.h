/** @file
    @brief	グラフィックス集合体描画

    @date	2017.05.04
    @author	Takashi SHUDO
*/

#ifndef GRAPHICS_OBJECT_H
#define GRAPHICS_OBJECT_H

/*
  グラフィックオブジェクト種別定義
*/
#define GO_TYPE_OBJECT_END	0	///< グラフィックオブジェクト配列終端
#define GO_TYPE_MODE		1	///< 描画モード設定
#define GO_TYPE_FORECOLOR	2	///< フォアカラー設定
#define GO_TYPE_BACKCOLOR	3	///< バックカラー設定
#define GO_TYPE_FONT		10	///< フォント設定
#define GO_TYPE_TEXT		20	///< 文字列
#define GO_TYPE_TEXT_IN_BOX	21	///< 四角形内文字列
#define GO_TYPE_FILL_CIRCLE	99	///< 塗りつぶした円
#define GO_TYPE_BOX		100	///< 四角
#define GO_TYPE_ROUND_BOX	101	///< 角の丸い四角
#define GO_TYPE_ROUND_FILL_BOX	102	///< 角の丸い塗りつぶした四角
#define GO_TYPE_FILL_BOX	103	///< 塗りつぶした四角
#define GO_TYPE_VERTEX4		104	///< 塗りつぶした4頂点の領域
#define GO_TYPE_SECTOR		105	///< 扇形

struct st_graph_object {
	int type;	///< グラフィックオブジェクト種別
	int arg[8];	///< グラフィックオブジェクト描画データ
	void *data;	///< グラフィックオブジェクトその他データ
}; ///< グラフィック集合体各要素

void set_graph_obj_scale(int numerator, int denominator);
void draw_graph_object(short x, short y, const struct st_graph_object *gobj);

#endif // GRAPHICS_OBJECT_H
