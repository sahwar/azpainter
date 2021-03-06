/*$
 Copyright (C) 2013-2018 Azel.

 This file is part of AzPainter.

 AzPainter is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 AzPainter is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
$*/

/*****************************************
 * mBufOperate
 *****************************************/

#include <string.h>

#include "mDef.h"
#include "mBufOperate.h"


/**
@defgroup bufoperate mBufOperate
@brief バッファ内のデータ操作
@ingroup group_data
 
@{
@file mBufOperate.h
@struct mBufOperate
@enum MBUFOPERATE_ENDIAN
*/


/** 初期化
 *
 * @param size 負の値で無制限 */

void mBufOpInit(mBufOperate *p,void *buf,int32_t size,uint8_t endian)
{
	p->cur = (uint8_t *)buf;
	p->top = p->cur;
	p->size = size;
	p->endian = endian;
}

/** 残りサイズ取得 */

int32_t mBufOpGetRemain(mBufOperate *p)
{
	return (p->size < 0)? INT32_MAX: p->size - (p->cur - p->top);
}

/** 残りのサイズが指定バイト以上あるか */

mBool mBufOpIsRemain(mBufOperate *p,int32_t size)
{
	return (p->size < 0 || p->size - (p->cur - p->top) >= size);
}

/** 指定サイズ移動 (＋方向のみ) */

mBool mBufOpSeek(mBufOperate *p,int32_t size)
{
	if(mBufOpIsRemain(p, size))
	{
		p->cur += size;
		return TRUE;
	}
	else
		return FALSE;
}

/** 位置をセット */

mBool mBufOpSetPos(mBufOperate *p,int32_t pos)
{
	if(pos < 0 || (p->size >= 0 && pos >= p->size))
		return FALSE;
	else
	{
		p->cur = p->top + pos;
		return TRUE;
	}
}

/** 位置をセット
 *
 * @return 変更前の位置を返す */

int32_t mBufOpSetPosRet(mBufOperate *p,int32_t pos)
{
	int32_t ret = p->cur - p->top;

	mBufOpSetPos(p, pos);
	return ret;
}


//=============================
// 読み込み
//=============================


/** 読み込み調整 */

static mBool _read(mBufOperate *p,void *buf,int32_t size)
{
	if(mBufOpIsRemain(p, size))
		return TRUE;
	else
	{
		//最後尾を超える場合、ゼロで埋める

		memset(buf, 0, size);

		p->cur = p->top + p->size;
	
		return FALSE;
	}
}

/** 読み込み
 *
 * @retval FALSE バッファのデータが足りない */

mBool mBufOpRead(mBufOperate *p,void *buf,int32_t size)
{
	if(!_read(p, buf, size))
		return FALSE;
	else
	{
		memcpy(buf, p->cur, size);
		p->cur += size;
		return TRUE;
	}
}

/** 1バイト読み込み */

mBool mBufOpReadByte(mBufOperate *p,void *buf)
{
	if(!_read(p, buf, 1))
		return FALSE;
	else
	{
		*((uint8_t *)buf) = *(p->cur);
		(p->cur)++;
		return TRUE;
	}
}

/** 16bit 値読み込み */

mBool mBufOpRead16(mBufOperate *p,void *buf)
{
	uint16_t v = 0;
	uint8_t *ps = p->cur;

	if(!_read(p, buf, 2)) return FALSE;

	switch(p->endian)
	{
		case MBUFOPERATE_ENDIAN_LITTLE:
			v = (ps[1] << 8) | ps[0];
			break;
		case MBUFOPERATE_ENDIAN_BIG:
			v = (ps[0] << 8) | ps[1];
			break;
		default:
			v = *((uint16_t *)ps);
			break;
	}

	*((uint16_t *)buf) = v;
	p->cur += 2;

	return TRUE;
}

/** 32bit 値読み込み */

mBool mBufOpRead32(mBufOperate *p,void *buf)
{
	uint32_t v = 0;
	uint8_t *ps = p->cur;

	if(!_read(p, buf, 4)) return FALSE;

	switch(p->endian)
	{
		case MBUFOPERATE_ENDIAN_LITTLE:
			v = ((uint32_t)ps[3] << 24) | (ps[2] << 16) | (ps[1] << 8) | ps[0];
			break;
		case MBUFOPERATE_ENDIAN_BIG:
			v = ((uint32_t)ps[0] << 24) | (ps[1] << 16) | (ps[2] << 8) | ps[3];
			break;
		default:
			v = *((uint32_t *)ps);
			break;
	}

	*((uint32_t *)buf) = v;
	p->cur += 4;

	return TRUE;
}

/** 16bit 値取得 */

uint16_t mBufOpGet16(mBufOperate *p)
{
	uint16_t v;

	mBufOpRead16(p, &v);
	return v;
}

/** 32bit 値取得 */

uint32_t mBufOpGet32(mBufOperate *p)
{
	uint32_t v;

	mBufOpRead32(p, &v);
	return v;
}


//=============================
// 書き込み
//=============================


/** データ書き込み */

void mBufOpWrite(mBufOperate *p,const void *buf,int32_t size)
{
	int remain = mBufOpGetRemain(p);

	if(size > remain) size = remain;

	memcpy(p->cur, buf, size);
	p->cur += size;
}

/** 16bit 数値書き込み */

void mBufOpWrite16(mBufOperate *p,void *buf)
{
	uint8_t b[2];
	uint16_t val = *((uint16_t *)buf);

	switch(p->endian)
	{
		case MBUFOPERATE_ENDIAN_LITTLE:
			b[0] = (uint8_t)val;
			b[1] = (uint8_t)(val >> 8);
			break;
		case MBUFOPERATE_ENDIAN_BIG:
			b[0] = (uint8_t)(val >> 8);
			b[1] = (uint8_t)val;
			break;
		default:
			*((uint16_t *)b) = val;
			break;
	}

	mBufOpWrite(p, b, 2);
}

/** 32bit 数値書き込み */

void mBufOpWrite32(mBufOperate *p,void *buf)
{
	uint8_t b[4];
	uint32_t val = *((uint32_t *)buf);

	switch(p->endian)
	{
		case MBUFOPERATE_ENDIAN_LITTLE:
			b[0] = (uint8_t)val;
			b[1] = (uint8_t)(val >> 8);
			b[2] = (uint8_t)(val >> 16);
			b[3] = (uint8_t)(val >> 24);
			break;
		case MBUFOPERATE_ENDIAN_BIG:
			b[0] = (uint8_t)(val >> 24);
			b[1] = (uint8_t)(val >> 16);
			b[2] = (uint8_t)(val >> 8);
			b[3] = (uint8_t)val;
			break;
		default:
			*((uint32_t *)b) = val;
			break;
	}

	mBufOpWrite(p, b, 4);
}

/** 16bit 数値書き込み */

void mBufOpSet16(mBufOperate *p,uint16_t val)
{
	mBufOpWrite16(p, &val);
}

/** 32bit 数値書き込み */

void mBufOpSet32(mBufOperate *p,uint32_t val)
{
	mBufOpWrite32(p, &val);
}

/** @} */
