#include "defines.h"
#include "serial.h"

#define SERIAL_SCI_NUM 3

//SCIの定義
#define H8_3069F_SCI0 ((volatile struct h8_3069f_sci *)0xffffb0)
#define H8_3069F_SCI1 ((volatile struct h8_3069f_sci *)0xffffb8)
#define H8_3069F_SCI2 ((volatile struct h8_3069f_sci *)0xffffc0)

//SCIの各種レジスタの定義
//シリアル通信速度（ボーレートは9600bps(1病患に9600ビット転送可能)）
struct h8_3069f_sci {
  volatile uint8 smr;    //シリアル通信のモード設定      シリアルモードレジスタ
  volatile uint8 brr;    //シリアル通信の速度の設定      ビットレートレジスタ
  volatile uint8 scr;    //送受信の有効/無効など         シリアルコントロールレジスタ
  volatile uint8 tdr;    //送信したい１文字を書き込む    トランスミットデータレジスタ
  volatile uint8 ssr;    //送信完了・受信完了などを表す  シリアルステータス・レジスタ
  volatile uint8 rdr;    //受信した１文字を読みだす      
  volatile uint8 scmr;
};

#define H8_3069F_SCI_SMR_CKS_PER1  (0<<0)
#define H8_3069F_SCI_SMR_CKS_PER4  (1<<0)
#define H8_3069F_SCI_SMR_CKS_PER16 (2<<0)
#define H8_3069F_SCI_SMR_CKS_PER64 (3<<0)
#define H8_3069F_SCI_SMR_MP     (1<<2)
#define H8_3069F_SCI_SMR_STOP   (1<<3)
#define H8_3069F_SCI_SMR_OE     (1<<4)
#define H8_3069F_SCI_SMR_PE     (1<<5)
#define H8_3069F_SCI_SMR_CHR    (1<<6)
#define H8_3069F_SCI_SMR_CA     (1<<7)

//SCRのビットの定義
#define H8_3069F_SCI_SCR_CKE0   (1<<0)
#define H8_3069F_SCI_SCR_CKE1   (1<<1)
#define H8_3069F_SCI_SCR_TEIE   (1<<2)
#define H8_3069F_SCI_SCR_MPIE   (1<<3)
#define H8_3069F_SCI_SCR_RE     (1<<4) /* 受信有効 */
#define H8_3069F_SCI_SCR_TE     (1<<5) /* 送信有効 */
#define H8_3069F_SCI_SCR_RIE    (1<<6) /* 受信割込み有効 */
#define H8_3069F_SCI_SCR_TIE    (1<<7) /* 送信割込み有効 */

//SSRのビットの定義
#define H8_3069F_SCI_SSR_MPBT   (1<<0)
#define H8_3069F_SCI_SSR_MPB    (1<<1)
#define H8_3069F_SCI_SSR_TEND   (1<<2)
#define H8_3069F_SCI_SSR_PER    (1<<3)
#define H8_3069F_SCI_SSR_FERERS (1<<4)
#define H8_3069F_SCI_SSR_ORER   (1<<5)
#define H8_3069F_SCI_SSR_RDRF   (1<<6) /* 受信完了 */
#define H8_3069F_SCI_SSR_TDRE   (1<<7) /* 送信完了 */

static struct {
  volatile struct h8_3069f_sci *sci;
} regs[SERIAL_SCI_NUM] = {
  { H8_3069F_SCI0 }, 
  { H8_3069F_SCI1 }, 
  { H8_3069F_SCI2 }, 
};

/* デバイス初期化 */
int serial_init(int index)
{
  volatile struct h8_3069f_sci *sci = regs[index].sci;

  sci->scr = 0;  //SCRにゼロを設定　シリアル送受信と割り込みをすべて無効化
  sci->smr = 0;
  sci->brr = 64; /* 20MHzのクロックから9600bpsを生成(25MHzの場合は80にする) */
  sci->scr = H8_3069F_SCI_SCR_RE | H8_3069F_SCI_SCR_TE; // 送受信を有効化
  sci->ssr = 0;

  return 0;
}

/* 送信可能の判断 */
int serial_is_send_enable(int index)
{
  volatile struct h8_3069f_sci *sci = regs[index].sci;
  return (sci->ssr & H8_3069F_SCI_SSR_TDRE);
}

/* １文字送信 */
int serial_send_byte(int index, unsigned char c)
{
  volatile struct h8_3069f_sci *sci = regs[index].sci;

  /* 送信可能になるまで待つ */
  //serial_is_send_enable()の戻り値を見て送信未完了の場合には完了するまで待つ
  while (!serial_is_send_enable(index))
    ;
  sci->tdr = c;    //TDRに送信文字を書き込み
  sci->ssr &= ~H8_3069F_SCI_SSR_TDRE; /* 送信開始 */

  return 0;
}
