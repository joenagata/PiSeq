// ************************************************** **************************************************
// PiSeq v1.0
//
// Pitch Interval Sequencer
// ************************************************** **************************************************
// <Hardware>
// M5Stack CoreS3 or SE
// Midi Unit with DIN Connector (SAM2695)
// 8-Encoder Unit (STM32F030)
// ************************************************** **************************************************
// <Boards>
// Arduino AVR Boards 1.8.6
// esp32 3.2.0
//
// <Library>
// ArduinoJson 7.4.0
// M5GFX 0.2.8
// M5UNIT_8Encoder 0.0.1
// M5Unified 0.2.6
// ************************************************** **************************************************
// ローカル変数    camelCase        dispPage
// グローバル変数  g_dispPage       g_dispPage
// 定数          UPPER_CASE        MAX_PAGE
// 列挙名（enum） PascalCase        PageType
// enumの中身    PascalCase        PagePlay
// ************************************************** **************************************************

const String SrcId = "PiSeq";
const String SrcVer = "1.0";

//#define Debug

#include <M5Unified.h>
#include <SD.h>             // SD Card
#include <ArduinoJson.h>    // Json
#include "M5_SAM2695.h"     // MIDI *** MIDI Clock, MIDI Start, MIDI Stopを追加 MIDI UnitのSWはSEPARATE
#include "UNIT_8ENCODER.h"  // 8encoder

#ifdef Debug
#include <FS.h>
#endif

#define SD_CS 4  // CoreS3のSDカードはCSピンがGPIO4

#define DarkRed 0x6004
#define DarkOrange 0x8200
#define DarkYellow 0x62E0
#define DarkGreen 0x0B20
#define DarkBlue 0x024C
#define DarkPurple 0x580C
#define DarkCyan 0x03EF

#define Red 0x9008
#define Orange 0xB1E0
#define Yellow 0xA480
#define Green 0x0460
#define Blue 0x01B0
#define Purple 0x9018
#define Cyan 0x07FF
#define White 0xFFFF
#define Black 0x0000
#define DarkGrey 0x7BEF

#define LedRed 0x070000
#define LedOrange 0x070300
#define LedYellow 0x070700
#define LedGreen 0x000700
#define LedBlue 0x000007
#define LedCyan 0x000707
#define LedPurple 0x070007

#define LedWhite 0x070707
#define LedBlack 0x000000

// ************************************************** **************************************************
// Const

enum MidiType {
  MidiStop = 0,
  MidiPause,
  MidiPlay
};

enum PageType {
  PagePlay = 0,
  PageSong,
  PageSeq,
  PageSeqRnd,
  PagePat,
  PagePatRnd,
  PageSet1,
  PageSet2,
  PageFile,
  PageLoad,
  PageSave,
  PageRename,
  PageDelete,
  PageInput
};

const char CharSet[65] = {
  'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
  'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
  'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '-', '_', ' '
};

const char *NoteName[12] = {
  "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

const char *GmInstruments[128] = {
  "Gr.Piano", "Br.Piano", "El.Gr.Piano", "Honky-tonk",
  "El.Piano 1", "El.Piano 2", "Harpsichord", "Clavinet",

  "Celesta", "Glocken", "Music Box", "Vibraphone",
  "Marimba", "Xylophone", "Tub Bells", "Dulcimer",

  "DB Organ", "Perc Organ", "Rock Organ", "Ch Organ",
  "Reed Organ", "Accordion", "Harmonica", "Accordion",

  "A.Gtr(nylon)", "A.Gtr(steel)", "E.Gtr(jazz)", "E.Gtr(clean)",
  "E.Gtr(mtd)", "OvrGtr", "DstGtr", "GtrHrm",

  "A.Bass", "E.Fing Bass", "E.Pick Bass", "Fl Bass",
  "Slap Bass1", "Slap Bass2", "Syn Bass1", "Syn Bass2",

  "Violin", "Viola", "Cello", "Contrabass",
  "TremStr", "PizzStr", "Harp", "Timpani",

  "StrEnsmbl1", "StrEnsmbl2", "Syn Str1", "Syn Str2",
  "Choir Aahs", "Voice Oohs", "Synth Choir", "Orch Hit",

  "Trumpet", "Trombone", "Tuba", "Mtd Tpt",
  "Fr.Horn", "Brass Sec", "Syn Brass1", "Syn Brass2",

  "Sopr Sax", "Alto Sax", "Tenor Sax", "Bari Sax",
  "Oboe", "Eng Horn", "Bassoon", "Clarinet",

  "Piccolo", "Flute", "Recorder", "Pan Flute",
  "Bottle", "Shakuhachi", "Whistle", "Ocarina",

  "Ld1(sqr)", "Ld2(saw)", "Ld3(callp)", "Ld4(chiff)",
  "Ld5(chrg)", "Ld6(voice)", "Ld7(fifths)", "Ld8(B+L)",

  "Pd1(new a)", "Pd2(warm)", "Pd3(poly)", "Pd4(choir)",
  "Pd5(bowed)", "Pd6(metal)", "Pd7(halo)", "Pd8(sweep)",

  "FX1(rain)", "FX2(sndtrk)", "FX3(crystal)", "FX4(atmos)",
  "FX5(brtns)", "FX6(goblin)", "FX7(echo)", "FX8(sci-fi)",

  "Sitar", "Banjo", "Shamisen", "Koto",
  "Kalimba", "Bagpipe", "Fiddle", "Shanai",

  "Tinkle Bell", "Agogo", "Steel Drum", "Woodblock",
  "Taiko Drum", "Mel Tom", "Synth Drum", "Rev Cymbal",

  "Fret Noise", "Brth Noise", "Seashore", "Bird Tweet",
  "Telephone", "Helicopter", "Applause", "Gunshot"
};

const char *ScaleName[] = {
  "Chrom", "Maj", "Min", "Harm Min", "Mel Min",
  "Maj Pent", "Min Pent", "Jpn", "Blues", "Whole Tone",
  "Diminished",
  //"Altered", "Lydian", "Dorian", "Phrygian", "Mixolydian", "Ionian",
  //"Aeolian", "Locrian",  "Arabic"
};

const char *NoteDurName[] = {
  "qtr.", "8th", "d.16", "16th", "qtr.", "hlf.", "whl."
};

const int ScaleNote[][12] = {
  { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 },      // Chrom
  { 0, 2, 4, 5, 7, 9, 11, -1, -1, -1, -1, -1 },  // Maj
  { 0, 2, 3, 5, 7, 8, 10, -1, -1, -1, -1, -1 },  // Min
  { 0, 2, 3, 5, 7, 8, 11, -1, -1, -1, -1, -1 },  // Harm Min
  { 0, 2, 3, 5, 7, 9, 11, -1, -1, -1, -1, -1 },  // Mel Min

  { 0, 2, 4, 7, 9, -1, -1, -1, -1, -1, -1, -1 },   // Maj Pent
  { 0, 3, 5, 7, 10, -1, -1, -1, -1, -1, -1, -1 },  // Min Pent
  { 0, 2, 3, 7, 9, -1, -1, -1, -1, -1, -1, -1 },   // Jpn
  { 0, 3, 5, 6, 7, 10, -1, -1, -1, -1, -1, -1 },   // Blues
  { 0, 2, 4, 6, 8, 10, -1, -1, -1, -1, -1, -1 },   // Whole Tone

  { 0, 1, 3, 4, 6, 7, 9, 10, -1, -1, -1, -1 },  // Diminished
};

const int ScaleNum[] = {
  12, 7, 7, 7, 7,
  5, 5, 5, 6, 6,
  8
};

const int MaxScale = 10;
const int MaxFiles = 100;

const int MaxOsc = 6;
const int MaxRtm = 4;
const int MaxSeq = 2;
const int SeqLen = 4;

// ************************************************** **************************************************
// Gloval

int g_playMode = MidiStop;  // MidiStop or MidiPuase or MidiPlay

int g_tempoBpm = 100;
long g_tempoUs = 2500000 / g_tempoBpm;  // 1 Tick = 1 Quarter note / 24

bool g_internalClock = true;
long g_tickCount = -1;
int g_rtmCount = 1;

int g_scale = 5;
int g_scaleNote[120];
int g_notePitch;

int g_key = 60;
int g_tuneKey = 60;
bool g_tuneOn = false;

int g_setChord[4] = { 0, 0, -3, -2 };
int g_setDur[4] = { 0, 2, 2, 2 };
int g_setPc[4] = { 0, 91, 91, 91 };
int g_setLevel[4] = { 30, 15, 15, 15 };
int g_setCh[4] = { 0, 1, 2, 3 };

int g_chordPitch[4];

int g_curSong = 0;  // Edit point
int g_curSeq = 0;
int g_curPat = 0;
int g_curRtm = 0;

int g_posSong = 0;  // Play point
int g_posSeq = 0;
int g_posPat = 0;
int g_posRtm = 0;

int g_posSongLast = 0;  // Play point
int g_posSeqLast = 0;
int g_posPatLast = 0;
int g_posRtmLast = 0;

int g_posRnd = 0;  // Rnd List point

int g_projSong[8];
int g_projLen = 1;

int g_songSeq[8][8];  // [0-7][0-7]
int g_songPitch[8][8];
int g_songLen[8] = { 1, 1, 1, 1, 1, 1, 1, 1 };

int g_seqPat[26][8];  // [0-25:A-Z][0-7]
int g_seqPitch[26][8];
int g_seqLen[26];

int g_seqRnd[26][8];
int g_seqRndPitch[26][7];

int g_seqRndList[8][17];
int g_seqRndPitchList[8][17];

int g_patRtm[26][4];  // [0-25:a-z][0-3]
int g_patPitch[26][4];
int g_patRtmDur[26];

int g_patRnd[26][8];
int g_patRndPitch[26][7];

int g_patRndList[8][17];
int g_patRndPitchList[8][17];

// User Interface

String g_Id = SrcId;
String g_Ver = SrcVer;

int g_curOsc = 0;

int g_curEncoder[8];

PageType g_page = PagePlay;
PageType g_caller;

int g_select = 0;

unsigned long g_startMillis;

String g_fileList[MaxFiles];
int g_fileCount = 1;  // 有効な最大のファイルを示す
int g_fileDisp = 0;

String g_curFile = "";
String g_oldFile = "";

String g_loadedFile = "********";

int g_namePos;
int g_nameChar;
int g_nameLen;
int g_nameData[16];

unsigned long g_prevTime = 0;

// **************************************************
// Touch
//   x, y	タッチされた位置
//   id	タッチID（複数タッチ対応用）
//   size	タッチのサイズ（大きさ）
//   wasPressed()	押された瞬間の検出
//   wasReleased()	離された瞬間の検出
//   isPressed()	押され続けているかどうか

auto g_touch = M5.Touch.getDetail();
static m5::touch_state_t g_prevTouch;

// **************************************************
// Timer
//   esp_timer_create()	タイマーを作成
//   esp_timer_start_once()	ワンショット（一度だけ）タイマー開始
//   esp_timer_start_periodic()	周期タイマー開始
//   esp_timer_stop()	タイマーを停止
//   esp_timer_delete()	タイマーを削除
//   esp_timer_get_time()	現在の時刻を取得（マイクロ秒）
//   esp_timer_is_active()	タイマーが有効か確認

esp_timer_handle_t g_timer;
bool g_timerTriggered = false;

// **************************************************
// MIDIユニット
//   begin() 初期化（I2C通信の初期化）
//   noteOn()	指定チャンネル・ノートを鳴らす
//   noteOff()	音を止める
//   programChange()	音色変更（0〜127）
//   pitchBend()	ピッチベンド
//   controlChange() CC
//   setVolume() CC#7
//   setPan() CC#10
//   setExpression() CC#11
//   allNotesOff() そのチャンネル上の全音を止める（CC#123）
//   reset() モジュールの初期化

M5_SAM2695 g_midi;

// **************************************************
// 8Encoderユニット
//   begin() 初期化（I2C通信の初期化）
//   getCounterValue()
//   setCounterValue()
//   getIncrementValue
//   getButtonStatus
//   getSwitchStatus
//   setLEDColor
//   setAllLEDColor
//   resetCounterValue

UNIT_8ENCODER g_encoder;

// ************************************************** **************************************************
// onTimer : 1Tick分の時間が経過したので、g_timerTriggeredを設定し、タイマーを再スタート
// ************************************************** **************************************************
void onTimer(void *arg) {

  if (g_playMode == MidiPlay) {
    g_tempoUs = 2500000 / g_tempoBpm;
    ESP_ERROR_CHECK(esp_timer_start_once(g_timer, g_tempoUs - 25));  // 25:オーバーヘッドの補正
  }

  g_timerTriggered = true;
}


// ************************************************** **************************************************
// evalTick : 1Tickごとの処理を実行
// ************************************************** **************************************************
void evalTick() {
  int seqPitch = 0;
  int patPitch = 0;
  int rtmPitch = 0;

  if (g_playMode == MidiPlay) {
    int t = ++g_tickCount % 24;  // 24 ticks = 1 quarter note

    if (t == 0) {
      M5.Display.fillCircle(5, 15, 5, WHITE);  // インジケーター点灯
      if (++g_posRnd > 16) {
        g_posRnd = 0;
      }
    } else if (t == 12) {
      M5.Display.fillCircle(5, 15, 5, BLACK);  // インジケーター消灯
    }

    if (--g_rtmCount == 0) {

      int song = g_projSong[g_posSong];

      int seq = g_songSeq[song][g_posSeq];
      seqPitch = g_songPitch[song][g_posSeq];
      if (seq >= 18) {  // Rnd
        seqPitch = g_seqRndPitchList[seq - 18][g_posRnd];
        seq = g_seqRndList[seq - 18][g_posRnd];
      }

      int pat = g_seqPat[seq][g_posPat];
      patPitch = g_seqPitch[seq][g_posPat];
      if (pat >= 18) {  // Rnd
        patPitch = g_patRndPitchList[pat - 18][g_posRnd];
        pat = g_patRndList[pat - 18][g_posRnd];
      }

      int rtm = g_patRtm[pat][g_posRtm];
      rtmPitch = g_patPitch[pat][g_posRtm];

      g_rtmCount = 24 / (g_patRtmDur[pat] + 1);

      g_notePitch = g_scaleNote[g_key + seqPitch + patPitch + rtmPitch];
      g_chordPitch[0] = g_notePitch;

      // Note On
      if (rtm == 1 && g_setLevel[0] > 0) {
        g_midi.setNoteOn(g_setCh[0], g_notePitch, g_setLevel[0]);
      }

      switch (g_page) {
        case PagePlay:
          M5.Display.drawRect(32 + g_posSongLast * 32, 165, 32, 30, Blue);

          M5.Display.fillRect(64, 45, 48, 30, Black);
          M5.Display.drawString(String(song + 1), 80 + 8, 60);
          M5.Display.fillRect(160, 45, 48, 30, Black);
          M5.Display.drawString(String((char)(seq + 'A')), 176 + 8, 60);
          M5.Display.fillRect(256, 45, 48, 30, Black);
          M5.Display.drawString(String((char)(pat + 'a')), 272 + 8, 60);

          M5.Display.drawRect(32 + g_posSong * 32, 165, 32, 30, White);
          break;
        case PageSong:
          M5.Display.drawRect(32 + g_posSeqLast * 32, 75, 32, 30, Purple);

          if (g_curSong != song) {
            g_curSong = song;
            M5.Display.fillRect(208, 0, 32, 30, Black);
            M5.Display.drawRect(208, 0, 32, 30, Blue);
            M5.Display.drawString(String(song + 1), 224, 15);

            M5.Display.fillRect(32, 75, 128, 30, Black);
            for (int i = 0; i < 8 && i < g_songLen[g_curSong]; i++) {
              M5.Display.drawRect(32 + i * 32, 75, 32, 30, Purple);
              M5.Display.drawString(String((char)(g_songSeq[g_curSong][i] + 'A')), 48 + i * 32, 91);
            }
            M5.Display.setFreeFont(&FreeSans9pt7b);
            M5.Display.fillRect(32, 150, 128, 30, Black);
            for (int i = 0; i < 8 && i < g_songLen[g_curSong]; i++) {
              M5.Display.drawRect(32 + i * 32, 150, 32, 30, Green);
              M5.Display.drawString(String(g_songPitch[g_curSong][i]), 48 + i * 32, 166);
            }
            M5.Display.setFreeFont(&FreeSans12pt7b);
          }

          M5.Display.drawRect(32 + g_posSeq * 32, 75, 32, 30, White);
          break;
        case PageSeq:
          M5.Display.drawRect(32 + g_posPatLast * 32, 75, 32, 30, Purple);

          if (g_curSeq != seq) {
            g_curSeq = seq;
            M5.Display.fillRect(208, 0, 32, 30, Black);
            M5.Display.drawRect(208, 0, 32, 30, Blue);
            M5.Display.drawString(String((char)(seq + 'A')), 224, 15);

            M5.Display.fillRect(32, 75, 128, 30, Black);
            for (int i = 0; i < 8 && i < g_seqLen[g_curSeq]; i++) {
              M5.Display.drawRect(32 + i * 32, 75, 32, 30, Purple);
              M5.Display.drawString(String((char)(g_seqPat[g_curSeq][i] + 'a')), 48 + i * 32, 91);
            }
            M5.Display.setFreeFont(&FreeSans9pt7b);
            M5.Display.fillRect(32, 150, 128, 30, Black);
            for (int i = 0; i < 8 && i < g_seqLen[g_curSeq]; i++) {
              M5.Display.drawRect(32 + i * 32, 150, 32, 30, Green);
              M5.Display.drawString(String(g_seqPitch[g_curSeq][i]), 48 + i * 32, 166);
            }
            M5.Display.setFreeFont(&FreeSans12pt7b);
          }

          M5.Display.drawRect(32 + g_posPat * 32, 75, 32, 30, White);
          break;
        case PagePat:
          M5.Display.drawRect(32 + g_posRtmLast * 32, 75, 32, 30, Purple);

          if (g_curPat != pat) {
            g_curPat = pat;
            M5.Display.fillRect(208, 0, 32, 30, Black);
            M5.Display.drawRect(208, 0, 32, 30, Blue);
            M5.Display.drawString(String((char)(pat + 'a')), 224, 15);

            M5.Display.fillRect(32, 75, 128, 30, Black);
            for (int i = 0; i <= g_patRtmDur[g_curPat]; i++) {
              M5.Display.drawRect(32 + i * 32, 75, 32, 30, Purple);
              M5.Display.drawString(String(g_patRtm[g_curPat][i]), 48 + i * 32, 91);
            }
            M5.Display.setFreeFont(&FreeSans9pt7b);
            M5.Display.fillRect(32, 150, 128, 30, Black);
            for (int i = 0; i <= g_patRtmDur[g_curPat]; i++) {
              M5.Display.drawRect(32 + i * 32, 150, 32, 30, Green);
              M5.Display.drawString(String(g_patPitch[g_curPat][i]), 48 + i * 32, 166);
            }
            M5.Display.setFreeFont(&FreeSans12pt7b);
          }

          M5.Display.drawRect(32 + g_posRtm * 32, 75, 32, 30, White);
          break;

        default:
          break;
      }

      // Set Next Note
      g_posRtmLast = g_posRtm;
      if (++g_posRtm > g_patRtmDur[pat]) {
        g_posPatLast = g_posPat;
        if (++g_posPat >= g_seqLen[seq]) {
          g_posSeqLast = g_posSeq;
          if (++g_posSeq >= g_songLen[song]) {
            g_posSongLast = g_posSong;
            if (++g_posSong >= g_projLen) {
              g_posSong = 0;  // 曲の終わりまでいったとき
            }
            g_posSeq = 0;
          }
          g_posPat = 0;
        }
        g_posRtm = 0;
      }
    } else if (g_rtmCount == 1) {
      // Note Off
      g_midi.setNoteOff(g_setCh[0], g_notePitch, g_setLevel[0]);
    }

    for (int i = 1; i < 4; i++) {
      if (g_tickCount % 24 == 0) {
        if (g_setDur[i] == 0 && g_setLevel[i] > 0) {
          g_chordPitch[i] = g_scaleNote[g_key + seqPitch + patPitch + rtmPitch + g_setChord[i]];
          g_midi.setNoteOn(g_setCh[i], g_chordPitch[i], g_setLevel[i]);
        }
      } else if (g_tickCount % 24 == 23) {
        if (g_setDur[i] == 0) {
          g_midi.setNoteOff(g_setCh[i], g_chordPitch[i], g_setLevel[i]);
        }
      }

      if (g_tickCount % 48 == 0) {
        if (g_setDur[i] == 1 && g_setLevel[i] > 0) {
          g_chordPitch[i] = g_scaleNote[g_key + seqPitch + patPitch + rtmPitch + g_setChord[i]];
          g_midi.setNoteOn(g_setCh[i], g_chordPitch[i], g_setLevel[i]);
        }
      } else if (g_tickCount % 48 == 47) {
        if (g_setDur[i] == 1) {
          g_midi.setNoteOff(g_setCh[i], g_chordPitch[i], g_setLevel[i]);
        }
      }

      if (g_tickCount % 96 == 0) {
        if (g_setDur[i] == 2 && g_setLevel[i] > 0) {
          g_chordPitch[i] = g_scaleNote[g_key + seqPitch + patPitch + rtmPitch + g_setChord[i]];
          g_midi.setNoteOn(g_setCh[i], g_chordPitch[i], g_setLevel[i]);
        }
      } else if (g_tickCount % 96 == 95) {
        if (g_setDur[i] == 2) {
          g_midi.setNoteOff(g_setCh[i], g_chordPitch[i], g_setLevel[i]);
        }
      }
    }
#ifdef Debug
    Serial.println("Tick:" + String(g_tickCount) + " " + String(t) + " rtm:" + g_rtmCount);
#endif
  }
}

// **************************************************
// getFileList() SDカードのファイル名の読み込み
// **************************************************
void getFileList() {

  if (!SD.begin(SD_CS)) {
    Serial.println("カードのマウントに失敗しました");
    return;
  }

  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("SDカードが挿入されていません");
    return;
  }

  File root = SD.open("/");
  if (!root || !root.isDirectory()) {
    Serial.println("ディレクトリを開けませんでした");
    return;
  }

  g_fileList[0] = "";
  g_fileList[1] = "";

  g_fileCount = 1;

  File file = root.openNextFile();
  while (file && g_fileCount < MaxFiles) {
    String filename = file.name();
    int k = filename.length();
    if (!file.isDirectory() && filename.charAt(0) != '.' && k > 8 && filename.substring(k - 8).equals("_pi.json")) {
      filename.remove(filename.length() - 8);
      g_fileList[++g_fileCount] = filename;
    }
    file = root.openNextFile();
  }
  root.close();

  // ファイル名のソート
  for (int i = 0; i < g_fileCount; i++) {
    for (int j = 0; j < g_fileCount - i; j++) {
      if (g_fileList[j] > g_fileList[j + 1]) {
        // ファイルを名を交換
        String temp = g_fileList[j];
        g_fileList[j] = g_fileList[j + 1];
        g_fileList[j + 1] = temp;
      }
    }
  }
}

// ************************************************** **************************************************
// checkTouch : タッチセンサー
// ************************************************** **************************************************
bool checkTouch() {
  g_touch = M5.Touch.getDetail();
  if (g_prevTouch != g_touch.state) {

    unsigned long currentMillis = millis();
    if (currentMillis - g_startMillis < 100) {  // タッチ後は0.1秒無視する
      return false;
    } else {
      g_startMillis = currentMillis;

      g_prevTouch = g_touch.state;

      return (g_touch.state == 1);
      // 0:none, 1:touch, 2:touch_end, 3:touch_begin,
      // 5:hold, 6:hold_end, 7:hold_begin,
      // 9:flick, 10:flick_end, 11:flick_begin,
      // 13:drag, 14:drag_end, 15:drag_begin
    }
  } else {
    return false;
  }
}

// ************************************************** **************************************************
// checkEncoder
// ************************************************** **************************************************
bool checkEncoder(int i, int min, int max, bool loop = false) {
  int k = g_encoder.getEncoderValue(i);

  if (k < min * 2) {
    k = loop ? ((k + (max - min + 1) * 2)) : (min * 2);
    k /= 2;
    k *= 2;
    g_encoder.setEncoderValue(i, k);
  } else if (k > max * 2) {
    k = loop ? ((k - (max - min + 1) * 2)) : (max * 2);
    k /= 2;
    k *= 2;
    g_encoder.setEncoderValue(i, k);
  }

  k /= 2;
  if (g_curEncoder[i] != k) {
    g_curEncoder[i] = k;
    return true;
  } else {
    return false;
  }
}

// ************************************************** **************************************************
// scrollList
// ************************************************** **************************************************
void scrollList() {
  if (checkEncoder(7, 0, g_fileCount - 2)) {
    g_fileDisp = g_curEncoder[7];
    M5.Display.fillRect(64, 30, 160, 180, BLACK);
    M5.Display.fillRect(64, 105, 160, 30, DarkYellow);
    for (int i = 0; i < 5 && i <= g_fileCount; i++) {
      if (i == 2) {
        g_curFile = g_fileList[i + g_fileDisp];
        M5.Display.setTextColor(WHITE, DarkYellow);
      } else {
        M5.Display.setTextColor(WHITE, BLACK);
      }
      M5.Display.drawString(g_fileList[i + g_fileDisp], 96, i * 30 + 61);
    }
  }
}

// ************************************************** **************************************************
// loadJson
// ************************************************** **************************************************
bool loadJson() {
  // ファイルオープン
  File file = SD.open("/" + g_curFile + "_pi.json");
  if (!file) {
    Serial.println("ファイルのオープンに失敗しました");
    return false;
  }

  // JSONデータを読み取る
  String jsonString;
  while (file.available()) {
    jsonString += (char)file.read();
  }
  file.close();

  Serial.println(g_curFile + "のJSONデータを読み込見ました: ");
  Serial.println(jsonString);

  // JSONを解析
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, jsonString);
  if (error) {
    Serial.print("JSON解析エラー: ");
    Serial.println(error.f_str());
    return false;
  }

  // 基本情報の読み出し
  g_Id = doc["id"] | "Other";
  g_Ver = doc["ver"] | "x.x";
  g_tempoBpm = doc["bpm"] | 100;
  g_internalClock = doc["int_clock"] | true;
  g_scale = doc["scale"] | 5;  // Maj Pent
  g_notePitch = doc["pitch"] | 0;
  g_key = doc["key"] | 60;
  g_projLen = doc["proj_len"] | 1;

  // 配列の読み出し

  JsonArray setChord_array = doc["set_chord"];
  JsonArray setDur_array = doc["set_dur"];
  JsonArray setPc_array = doc["set_pc"];
  JsonArray setLevel_array = doc["set_level"];
  JsonArray setCh_array = doc["set_ch"];
  JsonArray chordPitch_array = doc["chord_pitch"];
  for (int i = 0; i < 4; i++) {
    g_setChord[i] = setChord_array[i] | 0;
    g_setDur[i] = setDur_array[i] | 2;
    g_setPc[i] = setPc_array[i] | 0;
    g_setLevel[i] = setLevel_array[i] | 30;
    g_setCh[i] = setCh_array[i] | 0;
    g_chordPitch[i] = chordPitch_array[i] | 0;
  }

  JsonArray projSong_array = doc["proj_song"];
  JsonArray songLen_array = doc["song_len"];
  for (int i = 0; i < 8; i++) {
    g_projSong[i] = projSong_array[i] | 0;
    g_songLen[i] = songLen_array[i] | 1;
  }

  JsonArray songSeq_array = doc["song_seq"];
  JsonArray songPitch_array = doc["song_pitch"];
  for (int i = 0; i < 8; i++) {
    JsonArray row_songSeq = songSeq_array[i];
    JsonArray row_songPitch = songPitch_array[i];
    for (int j = 0; j < 8; j++) {
      g_songSeq[i][j] = row_songSeq[j] | 0;
      g_songPitch[i][j] = row_songPitch[j] | 0;
    }
  }

  JsonArray seqLen_array = doc["seq_len"];
  JsonArray patRtmDur_array = doc["pat_rtm_dur"];
  for (int i = 0; i < 26; i++) {
    g_seqLen[i] = seqLen_array[i] | 1;
    g_patRtmDur[i] = patRtmDur_array[i] | 0;
  }

  JsonArray seqPat_array = doc["seq_pat"];
  JsonArray seqPitch_array = doc["seq_pitch"];
  JsonArray seqRnd_array = doc["seq_rnd"];
  JsonArray patRnd_array = doc["pat_rnd"];
  for (int i = 0; i < 26; i++) {
    JsonArray row_seqPat = seqPat_array[i];
    JsonArray row_seqPitch = seqPitch_array[i];
    JsonArray row_seqRnd = seqRnd_array[i];
    JsonArray row_patRnd = patRnd_array[i];
    for (int j = 0; j < 8; j++) {
      g_seqPat[i][j] = row_seqPat[j] | 0;
      g_seqPitch[i][j] = row_seqPitch[j] | 0;
      g_seqRnd[i][j] = row_seqRnd[j] | 0;
      g_patRnd[i][j] = row_patRnd[j] | 0;
    }
  }

  JsonArray seqRndPitch_array = doc["seq_rnd_pitch"];
  JsonArray patRndPitch_array = doc["pat_rnd_pitch"];
  for (int i = 0; i < 26; i++) {
    JsonArray row_seqRndPitch = seqRndPitch_array[i];
    JsonArray row_patRndPitch = patRndPitch_array[i];
    for (int j = 0; j < 7; j++) {
      g_seqRndPitch[i][j] = row_seqRndPitch[j] | 0;
      g_patRndPitch[i][j] = row_patRndPitch[j] | 0;
    }
  }

  JsonArray seqRndList_array = doc["seq_rnd_list"];
  JsonArray seqRndPitchList_array = doc["seq_rnd_pitch_list"];
  JsonArray patRndList_array = doc["pat_rnd_list"];
  JsonArray patRndPitchList_array = doc["pat_rnd_pitch_list"];
  for (int i = 0; i < 8; i++) {
    JsonArray row_seqRndList = seqRndList_array[i];
    JsonArray row_seqRndPitchList = seqRndPitchList_array[i];
    JsonArray row_patRndList = patRndList_array[i];
    JsonArray row_patRndPitchList = patRndPitchList_array[i];
    for (int j = 0; j < 17; j++) {
      g_seqRndList[i][j] = row_seqRndList[j] | 0;
      g_seqRndPitchList[i][j] = row_seqRndPitchList[j] | 0;
      g_patRndList[i][j] = row_patRndList[j] | 0;
      g_patRndPitchList[i][j] = row_patRndPitchList[j] | 0;
    }
  }

  JsonArray patRtm_array = doc["pat_rtm"];
  JsonArray patPitch_array = doc["pat_pitch"];
  for (int i = 0; i < 26; i++) {
    JsonArray row_patRtm = patRtm_array[i];
    JsonArray row_patPitch = patPitch_array[i];
    for (int j = 0; j < 7; j++) {
      g_patRtm[i][j] = row_patRtm[j] | 0;
      g_patPitch[i][j] = row_patPitch[j] | 0;
    }
  }

  return true;
}

// ************************************************** **************************************************
// saveJson
// ************************************************** **************************************************
void saveJson() {
  int i, j;

  // JSONドキュメントの作成
  // サイズの計算 https://arduinojson.org/v6/assistant/
  //StaticJsonDocument<4096> doc;
  DynamicJsonDocument doc(49152);

  // 基本情報の格納
  doc["id"] = SrcId;
  doc["ver"] = SrcVer;
  doc["bpm"] = g_tempoBpm;
  doc["int_clock"] = g_internalClock;
  doc["scale"] = g_scale;
  doc["pitch"] = g_notePitch;
  doc["key"] = g_key;
  doc["proj_len"] = g_projLen;

  // 配列の書き込み

  JsonArray setChord_array = doc.createNestedArray("set_chord");
  JsonArray setDur_array = doc.createNestedArray("set_dur");
  JsonArray setPc_array = doc.createNestedArray("set_pc");
  JsonArray setLevel_array = doc.createNestedArray("set_level");
  JsonArray setCh_array = doc.createNestedArray("set_ch");
  JsonArray chordPitch_array = doc.createNestedArray("chord_pitch");
  for (i = 0; i < 4; i++) {
    setChord_array.add(g_setChord[i]);
    setDur_array.add(g_setDur[i]);
    setPc_array.add(g_setPc[i]);
    setLevel_array.add(g_setLevel[i]);
    setCh_array.add(g_setCh[i]);
    chordPitch_array.add(g_chordPitch[i]);
  }

  JsonArray projSong_array = doc.createNestedArray("proj_song");
  JsonArray songLen_array = doc.createNestedArray("song_len");
  for (i = 0; i < 8; i++) {
    projSong_array.add(g_projSong[i]);
    songLen_array.add(g_songLen[i]);
  }

  JsonArray songSeq_array = doc.createNestedArray("song_seq");
  JsonArray songPitch_array = doc.createNestedArray("song_pitch");
  for (i = 0; i < 8; i++) {
    JsonArray row_songSeq = songSeq_array.createNestedArray();
    JsonArray row_songPitch = songPitch_array.createNestedArray();
    for (j = 0; j < 8; j++) {
      row_songSeq.add(g_songSeq[i][j]);
      row_songPitch.add(g_songPitch[i][j]);
    }
  }

  JsonArray seqLen_array = doc.createNestedArray("seq_len");
  JsonArray patRtmDur_array = doc.createNestedArray("pat_rtm_dur");
  for (i = 0; i < 26; i++) {
    seqLen_array.add(g_seqLen[i]);
    patRtmDur_array.add(g_patRtmDur[i]);
  }

  JsonArray seqPat_array = doc.createNestedArray("seq_pat");
  JsonArray seqPitch_array = doc.createNestedArray("seq_pitch");
  JsonArray seqRnd_array = doc.createNestedArray("seq_rnd");
  JsonArray patRnd_array = doc.createNestedArray("pat_rnd");
  for (i = 0; i < 26; i++) {
    JsonArray row_seqPat = seqPat_array.createNestedArray();
    JsonArray row_seqPitch = seqPitch_array.createNestedArray();
    JsonArray row_seqRnd = seqRnd_array.createNestedArray();
    JsonArray row_patRnd = patRnd_array.createNestedArray();
    for (j = 0; j < 8; j++) {
      row_seqPat.add(g_seqPat[i][j]);
      row_seqPitch.add(g_seqPitch[i][j]);
      row_seqRnd.add(g_seqRnd[i][j]);
      row_patRnd.add(g_patRnd[i][j]);
    }
  }

  JsonArray seqRndPitch_array = doc.createNestedArray("seq_rnd_pitch");
  JsonArray patRndPitch_array = doc.createNestedArray("pat_rnd_pitch");
  for (i = 0; i < 26; i++) {
    JsonArray row_seqRndPitch = seqRndPitch_array.createNestedArray();
    JsonArray row_patRndPitch = patRndPitch_array.createNestedArray();
    for (j = 0; j < 7; j++) {
      row_seqRndPitch.add(g_seqRndPitch[i][j]);
      row_patRndPitch.add(g_patRndPitch[i][j]);
    }
  }

  JsonArray seqRndList_array = doc.createNestedArray("seq_rnd_list");
  JsonArray seqRndPitchList_array = doc.createNestedArray("seq_rnd_pitc_listh");
  JsonArray patRndList_array = doc.createNestedArray("pat_rnd_list");
  JsonArray patRndPitchList_array = doc.createNestedArray("pat_rnd_pitch_list");
  for (i = 0; i < 8; i++) {
    JsonArray row_seqRndList = seqRndList_array.createNestedArray();
    JsonArray row_seqRndPitchList = seqRndPitchList_array.createNestedArray();
    JsonArray row_patRndList = patRndList_array.createNestedArray();
    JsonArray row_patRndPitchList = patRndPitchList_array.createNestedArray();
    for (j = 0; j < 17; j++) {
      row_seqRndList.add(g_seqRndList[i][j]);
      row_seqRndPitchList.add(g_seqRndPitchList[i][j]);
      row_patRndList.add(g_patRndList[i][j]);
      row_patRndPitchList.add(g_patRndPitchList[i][j]);
    }
  }

  JsonArray patRtm_array = doc.createNestedArray("pat_rtm");
  JsonArray patPitch_array = doc.createNestedArray("pat_pitch");
  for (i = 0; i < 26; i++) {
    JsonArray row_patRtm = patRtm_array.createNestedArray();
    JsonArray row_patPitch = patPitch_array.createNestedArray();
    for (j = 0; j < 7; j++) {
      row_patRtm.add(g_patRtm[i][j]);
      row_patPitch.add(g_patPitch[i][j]);
    }
  }

  // JSONデータを文字列化
  String jsonString;
  serializeJson(doc, jsonString);
  //serializeJsonPretty(doc, jsonString); // 改行＆インデント付きで文字列化

  // SDカードに書き込む
  File file = SD.open("/" + g_curFile + "_pi.json", FILE_WRITE);
  if (file) {
    file.println(jsonString);
    file.close();
    Serial.println(g_curFile + "にJSONデータを書き込みました:");
    Serial.println(jsonString);
  } else {
    Serial.println("SDカードへの書き込みに失敗しました");
  }
}

// ************************************************** **************************************************
// setScale
// ************************************************** **************************************************
void setScale() {
  for (int i = 0; i < 120 - 60; i++) {
    int k = i / ScaleNum[g_scale];
    g_scaleNote[60 - i - 1] = 60 - k * 12 - 12 + ScaleNote[g_scale][ScaleNum[g_scale] - (i % ScaleNum[g_scale]) - 1];
    g_scaleNote[60 + i] = 60 + k * 12 + ScaleNote[g_scale][i % ScaleNum[g_scale]];
  }
}

// ************************************************** **************************************************
// dispPagePlay
// ************************************************** **************************************************
void dispPagePlay() {
  Serial.println("dispPagePlay");
  if (g_select > 1) {
    g_select = 0;
  }

  M5.Display.clear();
  M5.Display.setTextDatum(MC_DATUM);  // MC_DATUM : 中央、ML_DATUM : 左中央

  // Title
  M5.Display.setTextColor(GREENYELLOW, BLACK);
  M5.Display.drawString(SrcId, 64, 16);

  // Stop button
  M5.Display.fillRoundRect(192, 0, 64, 30, 4, DARKGREY);
  M5.Display.fillRect(214, 5, 20, 20, BLACK);

  // Play button
  if (g_playMode == 0) {
    M5.Display.fillRoundRect(256, 0, 64, 30, 4, DARKGREY);
    M5.Display.fillTriangle(278, 5, 298, 15, 278, 25, BLACK);
  } else if (g_playMode == 1) {
    M5.Display.fillRoundRect(256, 0, 64, 30, 4, DarkGreen);
    M5.Display.fillTriangle(278, 5, 298, 15, 278, 25, BLACK);
  } else {  // g_playMode == 2
    M5.Display.fillRoundRect(256, 0, 64, 30, 4, Green);
    M5.Display.fillRect(278, 5, 7, 20, BLACK);
    M5.Display.fillRect(291, 5, 7, 20, BLACK);
  }

  // Text
  M5.Display.setTextColor(White, Black);
  M5.Display.setFreeFont(&FreeSans9pt7b);
  M5.Display.setTextDatum(TR_DATUM);  // TR_DATUM : 右上

  M5.Display.drawString("Song", 64, 45);
  M5.Display.drawString("Seq", 160, 45);
  M5.Display.drawString("Pat", 256, 45);

  M5.Display.setTextDatum(TL_DATUM);  // TL_DATUM : 左上

  M5.Display.drawString("Tempo", 32, 90 - 4);
  M5.Display.drawString("Key", 96, 90 - 4);
  M5.Display.drawString("Scale", 160, 90 - 4);

  M5.Display.drawString("Song Order", 32, 150 - 4);

  M5.Display.setFreeFont(&FreeSans12pt7b);
  M5.Display.setTextDatum(MC_DATUM);  // MC_DATUM : 中央

  // Input area
  M5.Display.drawString(String(g_projSong[g_posSong] + 1), 80 + 8, 60);
  M5.Display.drawString(String((char)(g_songSeq[g_posSong][g_posSeq] + 'A')), 176 + 8, 60);
  M5.Display.drawString(String((char)(g_seqPat[g_posSeq][g_posPat] + 'a')), 272 + 8, 60);

  M5.Display.drawRect(32, 105, 64, 30, Blue);
  M5.Display.drawRect(96, 105, 64, 30, Red);
  M5.Display.drawRect(160, 105, 128, 30, Green);

  if (g_internalClock) {
    M5.Display.drawString(String(g_tempoBpm), 64, 120);
  } else {
    M5.Display.drawString("Ext", 64, 120);
  }

  M5.Display.drawString(String(NoteName[(g_key + 120) % 12]) + String((g_key >= 0) ? g_key / 12 - 1 : (g_key - 11) / 12 - 1), 128, 120);
  M5.Display.drawString(String(ScaleName[g_scale]), 224, 120);

  if (g_scale != 0) {
    String k = "";
    for (int i = 0; i < ScaleNum[g_scale]; i++) {
      k += String(NoteName[(g_key + g_scaleNote[60 + i]) % 12]) + " ";
    }
    M5.Display.setFreeFont(&FreeSans9pt7b);
    M5.Display.drawString(k, 224, 150 - 4);
    M5.Display.setFreeFont(&FreeSans12pt7b);
  }

  for (int i = 0; i < 8 && i < g_projLen; i++) {
    M5.Display.drawRect(32 + i * 32, 165, 32, 30, Blue);
    M5.Display.drawString(String(g_projSong[i] + 1), 48 + i * 32, 180);
  }

  // On/Off Button
  M5.Display.fillRoundRect(0, 105, 16, 30, 4, (g_select == 0) ? Orange : DarkGrey);
  M5.Display.fillRoundRect(0, 165, 16, 30, 4, (g_select == 1) ? Orange : DarkGrey);

  // Encoder
  if (g_select == 0) {
    g_encoder.setEncoderValue(7, g_tempoBpm * 2);
    g_curEncoder[7] = g_tempoBpm;
    delay(10);
    g_encoder.setEncoderValue(6, g_key * 2);
    g_curEncoder[6] = g_key;
    delay(10);
    g_encoder.setEncoderValue(5, g_scale * 2);
    g_curEncoder[5] = g_scale;
    delay(10);
  } else {
    for (int i = 0; i < 8; i++) {
      g_encoder.setEncoderValue(7 - i, g_projSong[i] * 2);
      g_curEncoder[7 - i] = g_projSong[i];
      delay(10);
    }
  }

  // Menu Button
  M5.Display.fillRoundRect(0, 210, 64, 30, 4, DarkRed);
  M5.Display.setTextColor(White, DarkRed);
  M5.Display.drawString("Song", 32, 226);

  M5.Display.fillRoundRect(64, 210, 64, 30, 4, DarkOrange);
  M5.Display.setTextColor(White, DarkOrange);
  M5.Display.drawString("Seq", 96, 226);

  M5.Display.fillRoundRect(128, 210, 64, 30, 4, DarkGreen);
  M5.Display.setTextColor(White, DarkGreen);
  M5.Display.drawString("Pat", 160, 226);

  M5.Display.fillRoundRect(192, 210, 64, 30, 4, DarkBlue);
  M5.Display.setTextColor(White, DarkBlue);
  M5.Display.drawString("Set", 224, 227);

  M5.Display.fillRoundRect(256, 210, 64, 30, 4, DarkPurple);
  M5.Display.setTextColor(White, DarkPurple);
  M5.Display.drawString("File", 288, 227);

  // LED
  if (g_select == 0) {
    g_encoder.setLEDColor(7, LedCyan);
    delay(10);
    g_encoder.setLEDColor(6, LedPurple);
    delay(10);
    g_encoder.setLEDColor(5, LedGreen);
    delay(10);
    g_encoder.setLEDColor(4, LedBlack);
    delay(10);
    g_encoder.setLEDColor(3, LedBlack);
    delay(10);
    g_encoder.setLEDColor(2, LedBlack);
    delay(10);
    g_encoder.setLEDColor(1, LedBlack);
    delay(10);
    g_encoder.setLEDColor(0, LedBlack);
    delay(10);
  } else {
    g_encoder.setLEDColor(7, LedCyan);
    delay(10);
    g_encoder.setLEDColor(6, LedCyan);
    delay(10);
    g_encoder.setLEDColor(5, LedCyan);
    delay(10);
    g_encoder.setLEDColor(4, LedCyan);
    delay(10);
    g_encoder.setLEDColor(3, LedCyan);
    delay(10);
    g_encoder.setLEDColor(2, LedCyan);
    delay(10);
    g_encoder.setLEDColor(1, LedCyan);
    delay(10);
    g_encoder.setLEDColor(0, LedCyan);
    delay(10);
  }
}

// ************************************************** **************************************************
// cntlPagePlay
// ************************************************** **************************************************
void cntlPagePlay() {
  if (checkTouch()) {
    Serial.println("cntlPagePlay - Touch");
    if (g_touch.y < 60) {  // タッチエリアをボタンより大きく設定
      if (g_touch.x >= 256) {
        if (g_playMode != MidiPlay) {
          // Play Button
          g_tempoUs = 2500000 / g_tempoBpm;
          ESP_ERROR_CHECK(esp_timer_start_once(g_timer, g_tempoUs));  // タイマーを開始
          g_playMode = MidiPlay;
          M5.Display.fillRoundRect(256, 0, 64, 30, 4, Green);
          M5.Display.fillRect(278, 5, 7, 20, BLACK);
          M5.Display.fillRect(291, 5, 7, 20, BLACK);
          for (int i = 0; i < 4; i++) {
            g_midi.setInstrument(0, g_setCh[i], g_setPc[i]);
          }

          if (g_internalClock) {
            g_midi.sendStart();
          }
          g_posSong = 0;
          g_posSeq = 0;
          g_posPat = 0;
          g_posRtm = 0;
          g_posSongLast = 0;
          g_posSeqLast = 0;
          g_posPatLast = 0;
          g_posRtmLast = 0;
        } else {
          // Pause Button
          g_playMode = MidiPause;
          M5.Display.fillRoundRect(256, 0, 64, 30, 4, DarkGreen);
          M5.Display.fillTriangle(278, 5, 298, 15, 278, 25, BLACK);

          M5.Display.fillCircle(5, 15, 5, BLACK);  // インジケーター消灯
          if (g_internalClock) {
            g_midi.sendStop();
          }
        }
      } else if (g_touch.x >= 192 && g_touch.x < 256) {
        // Stop Button
        g_tickCount = -1;
        g_rtmCount = 1;
        g_playMode = MidiStop;
        M5.Display.fillRoundRect(256, 0, 64, 30, 4, DARKGREY);
        M5.Display.fillTriangle(278, 5, 298, 15, 278, 25, BLACK);

        for (int i = 0; i < 4; i++) {  // Note Off
          g_midi.setNoteOff(g_setCh[i], g_chordPitch[i], g_setLevel[i]);
          //g_oscRing[i] = false;
          //}
        }

        M5.Display.fillCircle(5, 15, 5, BLACK);  // インジケーター消灯
        if (g_internalClock) {
          g_midi.sendStop();
        }
      }
    }

    // Select Button
    if (g_touch.x < 16 + 16) {
      if (g_touch.y >= 105 - 16 && g_touch.y < 135 + 16) {
        g_select = 0;
        M5.Display.fillRoundRect(0, 105, 16, 30, 4, Orange);
        M5.Display.fillRoundRect(0, 165, 16, 30, 4, DarkGrey);

        g_page = PagePlay;
        dispPagePlay();
        return;
      } else if (g_touch.y >= 165 - 16 && g_touch.y < 195 + 16) {
        g_select = 1;
        M5.Display.fillRoundRect(0, 105, 16, 30, 4, DarkGrey);
        M5.Display.fillRoundRect(0, 165, 16, 30, 4, Orange);

        g_page = PagePlay;
        dispPagePlay();
        return;
      }
    }

    if (g_touch.y >= 210) {
      if (g_touch.x < 64) {  // Song Button
        g_page = PageSong;
        dispPageSong();
        return;
      } else if (g_touch.x >= 64 && g_touch.x < 128) {  // Seq Button
        g_page = PageSeq;
        dispPageSeq();
        return;
      } else if (g_touch.x >= 128 && g_touch.x < 192) {  // Pat Button
        g_page = PagePat;
        dispPagePat();
        return;
      } else if (g_touch.x >= 192 && g_touch.x < 256) {  // Set Button
        g_page = PageSet1;
        dispPageSet1();
        return;
      } else if (g_touch.x >= 256) {  // File Button
        g_page = PageFile;
        dispPageFile();
        return;
      }
    }
  }

  // Encoder
  M5.Display.setTextColor(White, Black);
  if (g_select == 0) {
    if (g_internalClock) {
      if (checkEncoder(7, 40, 200)) {
        g_tempoBpm = g_curEncoder[7];
        M5.Display.fillRect(32, 105, 64, 30, Black);
        M5.Display.drawRect(32, 105, 64, 30, Blue);
        M5.Display.drawString(String(g_tempoBpm), 64, 120);
      }
    }
    if (checkEncoder(6, 0, 108)) {
      g_key = g_curEncoder[6];
      M5.Display.fillRect(96, 105, 64, 30, Black);
      M5.Display.drawRect(96, 105, 64, 30, Red);
      M5.Display.drawString(String(NoteName[(g_key + 120) % 12]) + String((g_key >= 0) ? g_key / 12 - 1 : (g_key - 11) / 12 - 1), 128, 120);

      g_page = PagePlay;
      dispPagePlay();
      return;
    }
    if (checkEncoder(5, 0, 10)) {
      g_scale = g_curEncoder[5];
      setScale();
      M5.Display.fillRect(160, 105, 128, 30, Black);
      M5.Display.drawRect(160, 105, 128, 30, Green);
      M5.Display.drawString(String(ScaleName[g_scale]), 224, 120);

      M5.Display.fillRect(128, 135, 192, 30, Black);
      if (g_scale != 0) {
        String k = "";
        for (int i = 0; i < ScaleNum[g_scale]; i++) {
          k += String(NoteName[(g_key + g_scaleNote[60 + i]) % 12]) + " ";
        }
        M5.Display.setFreeFont(&FreeSans9pt7b);
        M5.Display.drawString(k, 224, 150 - 4);
        M5.Display.setFreeFont(&FreeSans12pt7b);
      }
    }
    // Push Button
    if (!g_encoder.getButtonStatus(7)) {
      g_internalClock = !g_internalClock;
      g_page = PagePlay;
      dispPagePlay();
      return;
    }
  } else {
    for (int i = 0; i < 8; i++) {
      if (checkEncoder(7 - i, -1, 7)) {
        g_projSong[i] = g_curEncoder[7 - i];
        M5.Display.fillRect(32 + i * 32, 165, 32, 30, Black);
        M5.Display.drawRect(32 + i * 32, 165, 32, 30, Blue);
        if (g_projSong[i] >= 0) {
          M5.Display.drawString(String(g_projSong[i] + 1), 48 + i * 32, 180);
          if (g_projLen < i + 1) {
            g_projLen = i + 1;
            for (int j = 0; j < i; j++) {
              if (g_projSong[j] == -1) {
                g_projSong[j] = 0;
              }
            }
            g_page = PagePlay;
            dispPagePlay();
            return;
          }
        } else {
          if (g_projLen >= i + 1) {
            g_projLen = i;
            g_page = PagePlay;
            dispPagePlay();
            return;
          }
        }
      }
      // Push Button
      if (!g_encoder.getButtonStatus(7 - i)) {
        g_curSong = g_projSong[i];
        g_page = PageSong;
        dispPageSong();
        return;
      }
    }
  }

#ifdef Debug
  // Capture screen
  if (!g_encoder.getButtonStatus(0)) {
    delay(500);
    saveBMP("/disp_play.bmp");
    delay(500);
  }
#endif
}


// ************************************************** **************************************************
// dispPageSong
// ************************************************** **************************************************
void dispPageSong() {
  Serial.println("dispPageSong");

  M5.Display.clear();
  M5.Display.setTextDatum(MC_DATUM);  // MC_DATUM : 中央、ML_DATUM : 左中央

  // Title
  M5.Display.setTextColor(GREENYELLOW, BLACK);
  M5.Display.drawString(SrcId, 64, 16);

  // Text
  M5.Display.setTextColor(White, Black);

  M5.Display.drawString("Song", 176, 16);

  M5.Display.setFreeFont(&FreeSans9pt7b);
  M5.Display.setTextDatum(TL_DATUM);  // TL_DATUM : 左上

  M5.Display.drawString("Seq Order", 32, 60 - 4);
  M5.Display.drawString("Seq Pitch", 32, 135 - 4);

  M5.Display.setFreeFont(&FreeSans12pt7b);
  M5.Display.setTextDatum(MC_DATUM);  // MC_DATUM : 中央

  // Input area
  M5.Display.drawRect(208, 0, 32, 30, Blue);
  M5.Display.drawString(String(g_curSong + 1), 224, 15);

  for (int i = 0; i < 8 && i < g_songLen[g_curSong]; i++) {
    M5.Display.drawRect(32 + i * 32, 75, 32, 30, Purple);
    M5.Display.drawString(String((char)(g_songSeq[g_curSong][i] + 'A')), 48 + i * 32, 91);
  }

  M5.Display.setFreeFont(&FreeSans9pt7b);

  for (int i = 0; i < 8 && i < g_songLen[g_curSong]; i++) {
    M5.Display.drawRect(32 + i * 32, 150, 32, 30, Green);
    M5.Display.drawString(String(g_songPitch[g_curSong][i]), 48 + i * 32, 166);
  }

  M5.Display.setFreeFont(&FreeSans12pt7b);


  // On/Off Button
  M5.Display.fillRoundRect(128, 0, 16, 30, 4, (g_select == 0) ? Orange : DarkGrey);
  M5.Display.fillRoundRect(0, 75, 16, 30, 4, (g_select == 1) ? Orange : DarkGrey);
  M5.Display.fillRoundRect(0, 150, 16, 30, 4, (g_select == 2) ? Orange : DarkGrey);

  // Encoder
  if (g_select == 0) {
    g_encoder.setEncoderValue(7, g_curSong * 2);
    g_curEncoder[7] = g_curSong;
    delay(10);
  } else if (g_select == 1) {
    for (int i = 0; i < 8; i++) {
      g_encoder.setEncoderValue(7 - i, g_songSeq[g_curSong][i] * 2);
      g_curEncoder[7 - i] = g_songSeq[g_curSong][i];
      delay(10);
    }
  } else {
    for (int i = 0; i < 8; i++) {
      g_encoder.setEncoderValue(7 - i, g_songPitch[g_curSong][i] * 2);
      g_curEncoder[7 - i] = g_songPitch[g_curSong][i];
      delay(10);
    }
  }

  // Menu Button
  M5.Display.fillRoundRect(0, 210, 64, 30, 4, DarkGrey);
  M5.Display.setTextColor(White, DarkGrey);
  M5.Display.drawString("Home", 32, 226);

  M5.Display.fillRoundRect(80, 210, 64, 30, 4, DarkGrey);
  M5.Display.setTextColor(White, DarkGrey);
  M5.Display.drawString("Ret", 112, 226);

  // LED
  if (g_select == 0) {
    g_encoder.setLEDColor(7, LedCyan);
    delay(10);
    g_encoder.setLEDColor(6, LedBlack);
    delay(10);
    g_encoder.setLEDColor(5, LedBlack);
    delay(10);
    g_encoder.setLEDColor(4, LedBlack);
    delay(10);
    g_encoder.setLEDColor(3, LedBlack);
    delay(10);
    g_encoder.setLEDColor(2, LedBlack);
    delay(10);
    g_encoder.setLEDColor(1, LedBlack);
    delay(10);
    g_encoder.setLEDColor(0, LedBlack);
    delay(10);
  } else if (g_select == 1) {
    g_encoder.setLEDColor(7, LedPurple);
    delay(10);
    g_encoder.setLEDColor(6, LedPurple);
    delay(10);
    g_encoder.setLEDColor(5, LedPurple);
    delay(10);
    g_encoder.setLEDColor(4, LedPurple);
    delay(10);
    g_encoder.setLEDColor(3, LedPurple);
    delay(10);
    g_encoder.setLEDColor(2, LedPurple);
    delay(10);
    g_encoder.setLEDColor(1, LedPurple);
    delay(10);
    g_encoder.setLEDColor(0, LedPurple);
    delay(10);
  } else {
    g_encoder.setLEDColor(7, (0 < g_songLen[g_curSong]) ? LedGreen : LedBlack);
    delay(10);
    g_encoder.setLEDColor(6, (1 < g_songLen[g_curSong]) ? LedGreen : LedBlack);
    delay(10);
    g_encoder.setLEDColor(5, (2 < g_songLen[g_curSong]) ? LedGreen : LedBlack);
    delay(10);
    g_encoder.setLEDColor(4, (3 < g_songLen[g_curSong]) ? LedGreen : LedBlack);
    delay(10);
    g_encoder.setLEDColor(3, (4 < g_songLen[g_curSong]) ? LedGreen : LedBlack);
    delay(10);
    g_encoder.setLEDColor(2, (5 < g_songLen[g_curSong]) ? LedGreen : LedBlack);
    delay(10);
    g_encoder.setLEDColor(1, (6 < g_songLen[g_curSong]) ? LedGreen : LedBlack);
    delay(10);
    g_encoder.setLEDColor(0, (7 < g_songLen[g_curSong]) ? LedGreen : LedBlack);
    delay(10);
  }
}

// ************************************************** **************************************************
// cntlPageSong
// ************************************************** **************************************************
void cntlPageSong() {
  if (checkTouch()) {
    Serial.println("cntlPageSong - Touch");
    if (g_touch.y >= 210) {
      if (g_touch.x < 64) {  // Home Button
        g_page = PagePlay;
        dispPagePlay();
        return;
      } else if (g_touch.x >= 80 && g_touch.x < 144) {  // Return Button
        g_page = PagePlay;
        dispPagePlay();
        return;
      }
    }

    // Select Button
    if (g_touch.y < 30) {
      if (g_touch.x >= 128 - 16 && g_touch.x < 144 + 16) {
        g_select = 0;
        M5.Display.fillRoundRect(128, 0, 16, 30, 4, Orange);
        M5.Display.fillRoundRect(0, 75, 16, 30, 4, DarkGrey);
        M5.Display.fillRoundRect(0, 150, 16, 30, 4, DarkGrey);

        g_page = PageSong;
        dispPageSong();
        return;
      }
    } else if (g_touch.x < 16 + 16) {
      if (g_touch.y >= 75 - 16 && g_touch.y < 105 + 16) {
        g_select = 1;
        M5.Display.fillRoundRect(128, 0, 16, 30, 4, DarkGrey);
        M5.Display.fillRoundRect(0, 75, 16, 30, 4, Orange);
        M5.Display.fillRoundRect(0, 150, 16, 30, 4, DarkGrey);

        g_page = PageSong;
        dispPageSong();
        return;
      } else if (g_touch.y >= 150 - 16 && g_touch.y < 180 + 16) {
        g_select = 2;
        M5.Display.fillRoundRect(128, 0, 16, 30, 4, DarkGrey);
        M5.Display.fillRoundRect(0, 75, 16, 30, 4, DarkGrey);
        M5.Display.fillRoundRect(0, 150, 16, 30, 4, Orange);

        g_page = PageSong;
        dispPageSong();
        return;
      }
    }
  }

  // Encoder
  M5.Display.setTextColor(White, Black);
  if (g_select == 0) {
    if (checkEncoder(7, 0, 7)) {
      g_curSong = g_curEncoder[7];

      g_page = PageSong;
      dispPageSong();
      return;
    }
  } else if (g_select == 1) {
    for (int i = 0; i < 8; i++) {
      if (checkEncoder(7 - i, -1, 25)) {
        g_songSeq[g_curSong][i] = g_curEncoder[7 - i];
        M5.Display.fillRect(32 + i * 32, 75, 32, 30, Black);
        M5.Display.drawRect(32 + i * 32, 75, 32, 30, Purple);
        if (g_songSeq[g_curSong][i] >= 0) {
          M5.Display.drawString(String((char)(g_songSeq[g_curSong][i] + 'A')), 48 + i * 32, 90);
          if (g_songLen[g_curSong] < i + 1) {
            g_songLen[g_curSong] = i + 1;
            for (int j = 0; j < i; j++) {
              if (g_songSeq[g_curSong][j] == -1) {
                g_songSeq[g_curSong][j] = 0;
              }
            }
            g_page = PageSong;
            dispPageSong();
            return;
          }
        } else {
          if (g_songLen[g_curSong] >= i + 1) {
            g_songLen[g_curSong] = i;
            g_page = PageSong;
            dispPageSong();
            return;
          }
        }
      }
      // Push Button
      if (!g_encoder.getButtonStatus(7 - i)) {
        g_curSeq = g_songSeq[g_curSong][i];
        if (g_curSeq < 18) {
          g_page = PageSeq;
          dispPageSeq();
          return;
        } else {
          g_page = PageSeqRnd;
          dispPageSeqRnd();
          return;
        }
      }
    }
  } else {
    M5.Display.setFreeFont(&FreeSans9pt7b);
    for (int i = 0; i < 8 && i < g_songLen[g_curSong]; i++) {
      if (checkEncoder(7 - i, -24, 24)) {
        g_songPitch[g_curSong][i] = g_curEncoder[7 - i];
        M5.Display.fillRect(32 + i * 32, 150, 32, 30, Black);
        M5.Display.drawRect(32 + i * 32, 150, 32, 30, Green);

        M5.Display.drawString(String(g_songPitch[g_curSong][i]), 48 + i * 32, 165);
      }
    }
    M5.Display.setFreeFont(&FreeSans12pt7b);
  }

#ifdef Debug
  // Capture screen
  if (!g_encoder.getButtonStatus(0)) {
    delay(500);
    saveBMP("/disp_song.bmp");
    delay(500);
  }
#endif
}

// ************************************************** **************************************************
// dispPageSeq
// ************************************************** **************************************************
void dispPageSeq() {
  Serial.println("dispPageSeq");

  M5.Display.clear();
  M5.Display.setTextDatum(MC_DATUM);  // MC_DATUM : 中央、ML_DATUM : 左中央

  // Title
  M5.Display.setTextColor(GREENYELLOW, BLACK);
  M5.Display.drawString(SrcId, 64, 16);

  // Text
  M5.Display.setTextColor(White, Black);

  M5.Display.drawString("Seq", 176, 16);

  M5.Display.setFreeFont(&FreeSans9pt7b);
  M5.Display.setTextDatum(TL_DATUM);  // TL_DATUM : 左上

  M5.Display.drawString("Pat Order", 32, 60 - 4);
  M5.Display.drawString("Pat Pitch", 32, 135 - 4);

  M5.Display.setFreeFont(&FreeSans12pt7b);
  M5.Display.setTextDatum(MC_DATUM);  // MC_DATUM : 中央

  // Input area
  if (g_curSeq > 17) {
    g_curSeq = 0;
  }

  M5.Display.drawRect(208, 0, 32, 30, Blue);
  M5.Display.drawString(String((char)(g_curSeq + 'A')), 224, 15);

  for (int i = 0; i < 8 && i < g_seqLen[g_curSeq]; i++) {
    M5.Display.drawRect(32 + i * 32, 75, 32, 30, Purple);
    M5.Display.drawString(String((char)(g_seqPat[g_curSeq][i] + 'a')), 48 + i * 32, 91);
  }

  M5.Display.setFreeFont(&FreeSans9pt7b);

  for (int i = 0; i < 8 && i < g_seqLen[g_curSeq]; i++) {
    M5.Display.drawRect(32 + i * 32, 150, 32, 30, Green);
    M5.Display.drawString(String(g_seqPitch[g_curSeq][i]), 48 + i * 32, 166);
  }

  M5.Display.setFreeFont(&FreeSans12pt7b);

  // On/Off Button
  M5.Display.fillRoundRect(128, 0, 16, 30, 4, (g_select == 0) ? Orange : DarkGrey);
  M5.Display.fillRoundRect(0, 75, 16, 30, 4, (g_select == 1) ? Orange : DarkGrey);
  M5.Display.fillRoundRect(0, 150, 16, 30, 4, (g_select == 2) ? Orange : DarkGrey);

  // Encoder
  if (g_select == 0) {
    g_encoder.setEncoderValue(7, g_curSeq * 2);
    g_curEncoder[7] = g_curSeq;
    delay(10);
  } else if (g_select == 1) {
    for (int i = 0; i < 8; i++) {
      g_encoder.setEncoderValue(7 - i, g_seqPat[g_curSeq][i] * 2);
      g_curEncoder[7 - i] = g_seqPat[g_curSeq][i];
      delay(10);
    }
  } else {
    for (int i = 0; i < 8; i++) {
      g_encoder.setEncoderValue(7 - i, g_seqPitch[g_curSeq][i] * 2);
      g_curEncoder[7 - i] = g_seqPitch[g_curSeq][i];
      delay(10);
    }
  }

  // Menu Button
  M5.Display.fillRoundRect(0, 210, 64, 30, 4, DarkGrey);
  M5.Display.setTextColor(White, DarkGrey);
  M5.Display.drawString("Home", 32, 226);

  M5.Display.fillRoundRect(80, 210, 64, 30, 4, DarkGrey);
  M5.Display.setTextColor(White, DarkGrey);
  M5.Display.drawString("Ret", 112, 226);

  M5.Display.fillRoundRect(160, 210, 64, 30, 4, DarkGrey);
  M5.Display.setTextColor(White, DarkGrey);
  M5.Display.drawString("Rnd", 192, 226);

  // LED
  if (g_select == 0) {
    g_encoder.setLEDColor(7, LedCyan);
    delay(10);
    g_encoder.setLEDColor(6, LedBlack);
    delay(10);
    g_encoder.setLEDColor(5, LedBlack);
    delay(10);
    g_encoder.setLEDColor(4, LedBlack);
    delay(10);
    g_encoder.setLEDColor(3, LedBlack);
    delay(10);
    g_encoder.setLEDColor(2, LedBlack);
    delay(10);
    g_encoder.setLEDColor(1, LedBlack);
    delay(10);
    g_encoder.setLEDColor(0, LedBlack);
    delay(10);
  } else if (g_select == 1) {
    g_encoder.setLEDColor(7, LedPurple);
    delay(10);
    g_encoder.setLEDColor(6, LedPurple);
    delay(10);
    g_encoder.setLEDColor(5, LedPurple);
    delay(10);
    g_encoder.setLEDColor(4, LedPurple);
    delay(10);
    g_encoder.setLEDColor(3, LedPurple);
    delay(10);
    g_encoder.setLEDColor(2, LedPurple);
    delay(10);
    g_encoder.setLEDColor(1, LedPurple);
    delay(10);
    g_encoder.setLEDColor(0, LedPurple);
    delay(10);
  } else {
    g_encoder.setLEDColor(7, (0 < g_seqLen[g_curSeq]) ? LedGreen : LedBlack);
    delay(10);
    g_encoder.setLEDColor(6, (1 < g_seqLen[g_curSeq]) ? LedGreen : LedBlack);
    delay(10);
    g_encoder.setLEDColor(5, (2 < g_seqLen[g_curSeq]) ? LedGreen : LedBlack);
    delay(10);
    g_encoder.setLEDColor(4, (3 < g_seqLen[g_curSeq]) ? LedGreen : LedBlack);
    delay(10);
    g_encoder.setLEDColor(3, (4 < g_seqLen[g_curSeq]) ? LedGreen : LedBlack);
    delay(10);
    g_encoder.setLEDColor(2, (5 < g_seqLen[g_curSeq]) ? LedGreen : LedBlack);
    delay(10);
    g_encoder.setLEDColor(1, (6 < g_seqLen[g_curSeq]) ? LedGreen : LedBlack);
    delay(10);
    g_encoder.setLEDColor(0, (7 < g_seqLen[g_curSeq]) ? LedGreen : LedBlack);
    delay(10);
  }
}

// ************************************************** **************************************************
// cntlPageSeq
// ************************************************** **************************************************
void cntlPageSeq() {
  if (checkTouch()) {
    Serial.println("cntlPageSeq - Touch");
    if (g_touch.y >= 210) {
      if (g_touch.x < 64) {  // Home Button
        g_page = PagePlay;
        dispPagePlay();
        return;
      } else if (g_touch.x >= 80 && g_touch.x < 144) {  // Return Button
        g_page = PageSong;
        dispPageSong();
        return;
      } else if (g_touch.x >= 160 && g_touch.x < 224) {  // Rnd Button
        g_page = PageSeqRnd;
        dispPageSeqRnd();
        return;
      }
    }

    // Select Button
    if (g_touch.y < 30) {
      if (g_touch.x >= 128 - 16 && g_touch.x < 144 + 16) {
        g_select = 0;
        M5.Display.fillRoundRect(128, 0, 16, 30, 4, Orange);
        M5.Display.fillRoundRect(0, 75, 16, 30, 4, DarkGrey);
        M5.Display.fillRoundRect(0, 150, 16, 30, 4, DarkGrey);

        g_page = PageSeq;
        dispPageSeq();
        return;
      }
    } else if (g_touch.x < 16 + 16) {
      if (g_touch.y >= 75 - 16 && g_touch.y < 105 + 16) {
        g_select = 1;
        M5.Display.fillRoundRect(128, 0, 16, 30, 4, DarkGrey);
        M5.Display.fillRoundRect(0, 75, 16, 30, 4, Orange);
        M5.Display.fillRoundRect(0, 150, 16, 30, 4, DarkGrey);

        g_page = PageSeq;
        dispPageSeq();
        return;
      } else if (g_touch.y >= 150 - 16 && g_touch.y < 180 + 16) {
        g_select = 2;
        M5.Display.fillRoundRect(128, 0, 16, 30, 4, DarkGrey);
        M5.Display.fillRoundRect(0, 75, 16, 30, 4, DarkGrey);
        M5.Display.fillRoundRect(0, 150, 16, 30, 4, Orange);

        g_page = PageSeq;
        dispPageSeq();
        return;
      }
    }
  }

  // Encoder
  M5.Display.setTextColor(White, Black);
  if (g_select == 0) {
    if (checkEncoder(7, 0, 17, true)) {
      g_curSeq = g_curEncoder[7];

      g_page = PageSeq;
      dispPageSeq();
      return;
    }
  } else if (g_select == 1) {
    for (int i = 0; i < 8; i++) {
      if (checkEncoder(7 - i, -1, 25)) {
        g_seqPat[g_curSeq][i] = g_curEncoder[7 - i];
        M5.Display.fillRect(32 + i * 32, 75, 32, 30, Black);
        M5.Display.drawRect(32 + i * 32, 75, 32, 30, Purple);
        if (g_seqPat[g_curSeq][i] >= 0) {
          M5.Display.drawString(String((char)(g_seqPat[g_curSeq][i] + 'a')), 48 + i * 32, 90);
          if (g_seqLen[g_curSeq] < i + 1) {
            g_seqLen[g_curSeq] = i + 1;
            for (int j = 0; j < i; j++) {
              if (g_seqPat[g_curSeq][j] == -1) {
                g_seqPat[g_curSeq][j] = 0;
              }
            }
            g_page = PageSeq;
            dispPageSeq();
            return;
          }
        } else {
          if (g_seqLen[g_curSeq] >= i + 1) {
            g_seqLen[g_curSeq] = i;
            g_page = PageSeq;
            dispPageSeq();
            return;
          }
        }
      }
      // Push Button
      if (!g_encoder.getButtonStatus(7 - i)) {
        g_curPat = g_seqPat[g_curSeq][i];
        if (g_curPat < 18) {
          g_page = PagePat;
          dispPagePat();
          return;
        } else {
          g_page = PagePatRnd;
          dispPagePatRnd();
          return;
        }
      }
    }
  } else {
    M5.Display.setFreeFont(&FreeSans9pt7b);
    for (int i = 0; i < 8 && i < g_seqLen[g_curSeq]; i++) {
      if (checkEncoder(7 - i, -24, 24)) {
        g_seqPitch[g_curSeq][i] = g_curEncoder[7 - i];
        M5.Display.fillRect(32 + i * 32, 150, 32, 30, Black);
        M5.Display.drawRect(32 + i * 32, 150, 32, 30, Green);

        M5.Display.drawString(String(g_seqPitch[g_curSeq][i]), 48 + i * 32, 165);
      }
    }
    M5.Display.setFreeFont(&FreeSans12pt7b);
  }

#ifdef Debug
  // Capture screen
  if (!g_encoder.getButtonStatus(0)) {
    delay(500);
    saveBMP("/disp_seq.bmp");
    delay(500);
  }
#endif
}

// ************************************************** **************************************************
// genSeqRnd
// ************************************************** **************************************************
void genSeqRnd() {

  // g_seqRnd[g_curSeq - 18][i] // Seq選択肢の重み
  // g_seqRndList[g_curSeq - 18][j] // 乱数１６個を格納
  int totalWeight = 0;

  // 総重みを計算
  for (int i = 0; i < 8; i++) {
    totalWeight += g_seqRnd[g_curSeq - 18][i];
  }

  for (int j = 0; j < 17; j++) {
    int r = random(0, totalWeight);
    int sum = 0;
    int i;

    for (i = 0; i < 8; i++) {
      sum += g_seqRnd[g_curSeq - 18][i];
      if (r < sum) {
        break;
      }
    }
    if (i == 8) {  // 重みがAll ０の場合
      //i = 0;
      i = random(0, 8);
    }
    g_seqRndList[g_curSeq - 18][j] = i;
  }

  // g_seqRndPitch[g_curSeq - 18][i]  // SeqのPitch選択肢の重み
  // g_seqRndPitchList[g_curSeq - 18][i]  // 乱数１６個を格納
  totalWeight = 0;

  // 総重みを計算
  for (int i = 0; i < 7; i++) {
    totalWeight += g_seqRndPitch[g_curSeq - 18][i];
  }

  for (int j = 0; j < 17; j++) {
    int r = random(0, totalWeight);
    int sum = 0;
    int i;

    for (i = 0; i < 7; i++) {
      sum += g_seqRndPitch[g_curSeq - 18][i];
      if (r < sum) {
        break;
      }
    }
    if (i == 7) {  // 重みがAll ０の場合
      //i = 3;
      i = random(0, 7);
    }
    g_seqRndPitchList[g_curSeq - 18][j] = i;
  }

  Serial.println("genSeqRnd " + String((char)(g_curSeq + 'A')));
  for (int i = 0; i < 17; i++) {
    Serial.print(" " + String((char)(g_seqRndList[g_curSeq - 18][i] + 'A')));
  }
  Serial.println();
  for (int i = 0; i < 17; i++) {
    Serial.print(" " + String(g_seqRndPitchList[g_curSeq - 18][i]));
  }
  Serial.println();
}

// ************************************************** **************************************************
// dispPageSeqRnd
// ************************************************** **************************************************
void dispPageSeqRnd() {
  Serial.println("dispPageSeqRnd");

  M5.Display.clear();
  M5.Display.setTextDatum(MC_DATUM);  // MC_DATUM : 中央、ML_DATUM : 左中央

  // Title
  M5.Display.setTextColor(GREENYELLOW, BLACK);
  M5.Display.drawString(SrcId, 64, 16);

  // Text
  M5.Display.setTextColor(White, Black);

  M5.Display.drawString("Seq", 176, 16);
  M5.Display.drawString("Rnd", 272, 16);

  M5.Display.setFreeFont(&FreeSans9pt7b);
  M5.Display.setTextDatum(TL_DATUM);  // TL_DATUM : 左上

  M5.Display.drawString("Seq Probability", 32, 30 + 4);
  M5.Display.drawString("Pitch Probability", 32, 105 + 4);

  M5.Display.setTextDatum(MC_DATUM);  // MC_DATUM : 中央

  for (int i = 0; i < 8; i++) {
    M5.Display.drawString(String((char)('A' + i)), 48 + i * 32, 60 + 4);
    if (i < 7) {
      M5.Display.drawString(String(i - 3), 48 + i * 32, 135 + 4);
    }
  }

  M5.Display.setFreeFont(&FreeSans12pt7b);

  // Input area
  if (g_curSeq < 18) {
    g_curSeq = 18;
  }

  M5.Display.drawRect(208, 0, 32, 30, Blue);
  M5.Display.drawString(String((char)(g_curSeq + 'A')), 224, 15);

  for (int i = 0; i < 8; i++) {
    M5.Display.drawRect(32 + i * 32, 75, 32, 30, Purple);
    M5.Display.drawString(String(g_seqRnd[g_curSeq - 18][i]), 48 + i * 32, 91);
  }

  for (int i = 0; i < 7; i++) {
    M5.Display.drawRect(32 + i * 32, 150, 32, 30, Green);
    M5.Display.drawString(String(g_seqRndPitch[g_curSeq - 18][i]), 48 + i * 32, 166);
  }

  // On/Off Button
  M5.Display.fillRoundRect(128, 0, 16, 30, 4, (g_select == 0) ? Orange : DarkGrey);
  M5.Display.fillRoundRect(0, 75, 16, 30, 4, (g_select == 1) ? Orange : DarkGrey);
  M5.Display.fillRoundRect(0, 150, 16, 30, 4, (g_select == 2) ? Orange : DarkGrey);

  // Encoder
  if (g_select == 0) {
    g_encoder.setEncoderValue(7, g_curSeq * 2);
    g_curEncoder[7] = g_curSeq;
    delay(10);
  } else if (g_select == 1) {
    for (int i = 0; i < 8; i++) {
      g_encoder.setEncoderValue(7 - i, g_seqRnd[g_curSeq - 18][i] * 2);
      g_curEncoder[7 - i] = g_seqRnd[g_curSeq - 18][i];
      delay(10);
    }
  } else {
    for (int i = 0; i < 8; i++) {
      g_encoder.setEncoderValue(7 - i, g_seqRndPitch[g_curSeq - 18][i] * 2);
      g_curEncoder[7 - i] = g_seqRndPitch[g_curSeq - 18][i];
      delay(10);
    }
  }

  // Menu Button
  M5.Display.fillRoundRect(0, 210, 64, 30, 4, DarkGrey);
  M5.Display.setTextColor(White, DarkGrey);
  M5.Display.drawString("Home", 32, 226);

  M5.Display.fillRoundRect(80, 210, 64, 30, 4, DarkGrey);
  M5.Display.setTextColor(White, DarkGrey);
  M5.Display.drawString("Ret", 112, 226);

  M5.Display.fillRoundRect(160, 210, 64, 30, 4, DarkGrey);
  M5.Display.setTextColor(White, DarkGrey);
  M5.Display.drawString("Std", 192, 226);

  // LED
  if (g_select == 0) {
    g_encoder.setLEDColor(7, LedCyan);
    delay(10);
    g_encoder.setLEDColor(6, LedBlack);
    delay(10);
    g_encoder.setLEDColor(5, LedBlack);
    delay(10);
    g_encoder.setLEDColor(4, LedBlack);
    delay(10);
    g_encoder.setLEDColor(3, LedBlack);
    delay(10);
    g_encoder.setLEDColor(2, LedBlack);
    delay(10);
    g_encoder.setLEDColor(1, LedBlack);
    delay(10);
    g_encoder.setLEDColor(0, LedBlack);
    delay(10);
  } else if (g_select == 1) {
    g_encoder.setLEDColor(7, LedPurple);
    delay(10);
    g_encoder.setLEDColor(6, LedPurple);
    delay(10);
    g_encoder.setLEDColor(5, LedPurple);
    delay(10);
    g_encoder.setLEDColor(4, LedPurple);
    delay(10);
    g_encoder.setLEDColor(3, LedPurple);
    delay(10);
    g_encoder.setLEDColor(2, LedPurple);
    delay(10);
    g_encoder.setLEDColor(1, LedPurple);
    delay(10);
    g_encoder.setLEDColor(0, LedPurple);
    delay(10);
  } else {
    g_encoder.setLEDColor(7, LedGreen);
    delay(10);
    g_encoder.setLEDColor(6, LedGreen);
    delay(10);
    g_encoder.setLEDColor(5, LedGreen);
    delay(10);
    g_encoder.setLEDColor(4, LedGreen);
    delay(10);
    g_encoder.setLEDColor(3, LedGreen);
    delay(10);
    g_encoder.setLEDColor(2, LedGreen);
    delay(10);
    g_encoder.setLEDColor(1, LedGreen);
    delay(10);
    g_encoder.setLEDColor(0, LedBlack);
    delay(10);
  }
}

// ************************************************** **************************************************
// cntlPageSeqRnd
// ************************************************** **************************************************
void cntlPageSeqRnd() {
  if (checkTouch()) {
    Serial.println("cntlPageSeqRnd - Touch");
    if (g_touch.y >= 210) {
      if (g_touch.x < 64) {  // Home Button
        genSeqRnd();
        g_page = PagePlay;
        dispPagePlay();
        return;
      } else if (g_touch.x >= 80 && g_touch.x < 144) {  // Return Button
        genSeqRnd();
        g_page = PageSong;
        dispPageSong();
        return;
      } else if (g_touch.x >= 160 && g_touch.x < 224) {  // Std Button
        genSeqRnd();
        g_page = PageSeq;
        dispPageSeq();
        return;
      }
    }

    // Select Button
    if (g_touch.y < 30) {
      if (g_touch.x >= 128 - 16 && g_touch.x < 144 + 16) {
        g_select = 0;
        M5.Display.fillRoundRect(128, 0, 16, 30, 4, Orange);
        M5.Display.fillRoundRect(0, 75, 16, 30, 4, DarkGrey);
        M5.Display.fillRoundRect(0, 150, 16, 30, 4, DarkGrey);

        g_page = PageSeqRnd;
        dispPageSeqRnd();
        return;
      }
    } else if (g_touch.x < 16 + 16) {
      if (g_touch.y >= 75 - 16 && g_touch.y < 105 + 16) {
        g_select = 1;
        M5.Display.fillRoundRect(128, 0, 16, 30, 4, DarkGrey);
        M5.Display.fillRoundRect(0, 75, 16, 30, 4, Orange);
        M5.Display.fillRoundRect(0, 150, 16, 30, 4, DarkGrey);

        g_page = PageSeqRnd;
        dispPageSeqRnd();
        return;
      } else if (g_touch.y >= 150 - 16 && g_touch.y < 180 + 16) {
        g_select = 2;
        M5.Display.fillRoundRect(128, 0, 16, 30, 4, DarkGrey);
        M5.Display.fillRoundRect(0, 75, 16, 30, 4, DarkGrey);
        M5.Display.fillRoundRect(0, 150, 16, 30, 4, Orange);

        g_page = PageSeqRnd;
        dispPageSeqRnd();
        return;
      }
    }
  }

  // Encoder
  M5.Display.setTextColor(White, Black);
  if (g_select == 0) {
    if (checkEncoder(7, 18, 25, true)) {
      genSeqRnd();
      g_curSeq = g_curEncoder[7];
      g_page = PageSeqRnd;
      dispPageSeqRnd();
      return;
    }
  } else if (g_select == 1) {
    for (int i = 0; i < 8; i++) {
      if (checkEncoder(7 - i, 0, 9)) {
        g_seqRnd[g_curSeq - 18][i] = g_curEncoder[7 - i];
        M5.Display.fillRect(32 + i * 32, 75, 32, 30, Black);
        M5.Display.drawRect(32 + i * 32, 75, 32, 30, Purple);
        M5.Display.drawString(String(g_seqRnd[g_curSeq - 18][i]), 48 + i * 32, 90);
      }
    }
  } else {
    for (int i = 0; i < 7; i++) {
      if (checkEncoder(7 - i, 0, 9)) {
        g_seqRndPitch[g_curSeq - 18][i] = g_curEncoder[7 - i];
        M5.Display.fillRect(32 + i * 32, 150, 32, 30, Black);
        M5.Display.drawRect(32 + i * 32, 150, 32, 30, Green);
        M5.Display.drawString(String(g_seqRndPitch[g_curSeq - 18][i]), 48 + i * 32, 165);
      }
    }
  }

#ifdef Debug
  // Capture screen
  if (!g_encoder.getButtonStatus(0)) {
    delay(500);
    saveBMP("/disp_seqrnd.bmp");
    delay(500);
  }
#endif
}

// ************************************************** **************************************************
// dispPagePat
// ************************************************** **************************************************
void dispPagePat() {
  Serial.println("dispPagePat");

  M5.Display.clear();
  M5.Display.setTextDatum(MC_DATUM);  // MC_DATUM : 中央、ML_DATUM : 左中央

  // Title
  M5.Display.setTextColor(GREENYELLOW, BLACK);
  M5.Display.drawString(SrcId, 64, 16);

  // Text
  M5.Display.setTextColor(White, Black);

  M5.Display.drawString("Pat", 176, 16);

  M5.Display.setFreeFont(&FreeSans9pt7b);
  M5.Display.setTextDatum(TL_DATUM);  // TL_DATUM : 左上

  M5.Display.drawString("Rtm", 32, 60 - 4);
  M5.Display.drawString("Note", 208, 60 - 4);
  M5.Display.drawString("Pitch", 32, 135 - 4);

  M5.Display.setFreeFont(&FreeSans12pt7b);
  M5.Display.setTextDatum(MC_DATUM);  // MC_DATUM : 中央

  // Input area
  if (g_curPat > 17) {
    g_curPat = 0;
  }

  // Input area
  M5.Display.drawRect(208, 0, 32, 30, Blue);
  M5.Display.drawString(String((char)(g_curPat + 'a')), 224, 15);

  M5.Display.drawRect(208, 75, 32, 30, Blue);
  M5.Display.drawString(String(g_patRtmDur[g_curPat] + 1), 224, 90);

  M5.Display.drawString(String(NoteDurName[g_patRtmDur[g_curPat]]), 272, 90);



  for (int i = 0; i <= g_patRtmDur[g_curPat]; i++) {
    M5.Display.drawRect(32 + i * 32, 75, 32, 30, Purple);
    M5.Display.drawString(String(g_patRtm[g_curPat][i]), 48 + i * 32, 91);
  }

  M5.Display.setFreeFont(&FreeSans9pt7b);

  for (int i = 0; i <= g_patRtmDur[g_curPat]; i++) {
    M5.Display.drawRect(32 + i * 32, 150, 32, 30, Green);
    M5.Display.drawString(String(g_patPitch[g_curPat][i]), 48 + i * 32, 166);
  }

  M5.Display.setFreeFont(&FreeSans12pt7b);

  // On/Off Button
  M5.Display.fillRoundRect(128, 0, 16, 30, 4, (g_select == 0) ? Orange : DarkGrey);
  M5.Display.fillRoundRect(0, 75, 16, 30, 4, (g_select == 1) ? Orange : DarkGrey);
  M5.Display.fillRoundRect(0, 150, 16, 30, 4, (g_select == 2) ? Orange : DarkGrey);

  // Encoder
  if (g_select == 0) {
    g_encoder.setEncoderValue(7, g_curPat * 2);
    g_curEncoder[7] = g_curPat;
    delay(10);
    g_encoder.setEncoderValue(6, g_patRtmDur[g_curPat] * 2);
    g_curEncoder[6] = g_patRtmDur[g_curPat];
    delay(10);
  } else if (g_select == 1) {
    for (int i = 0; i < 4; i++) {
      g_encoder.setEncoderValue(7 - i, g_patRtm[g_curPat][i] * 2);
      g_curEncoder[7 - i] = g_patRtm[g_curPat][i];
      delay(10);
    }
  } else {
    for (int i = 0; i < 4; i++) {
      g_encoder.setEncoderValue(7 - i, g_patPitch[g_curPat][i] * 2);
      g_curEncoder[7 - i] = g_patPitch[g_curPat][i];
      delay(10);
    }
  }

  // Menu Button
  M5.Display.fillRoundRect(0, 210, 64, 30, 4, DarkGrey);
  M5.Display.setTextColor(White, DarkGrey);
  M5.Display.drawString("Home", 32, 226);

  M5.Display.fillRoundRect(80, 210, 64, 30, 4, DarkGrey);
  M5.Display.setTextColor(White, DarkGrey);
  M5.Display.drawString("Ret", 112, 226);

  M5.Display.fillRoundRect(160, 210, 64, 30, 4, DarkGrey);
  M5.Display.setTextColor(White, DarkGrey);
  M5.Display.drawString("Rnd", 192, 226);

  // LED
  if (g_select == 0) {
    g_encoder.setLEDColor(7, LedCyan);
    delay(10);
    g_encoder.setLEDColor(6, LedCyan);
    delay(10);
    g_encoder.setLEDColor(5, LedBlack);
    delay(10);
    g_encoder.setLEDColor(4, LedBlack);
    delay(10);
    g_encoder.setLEDColor(3, LedBlack);
    delay(10);
    g_encoder.setLEDColor(2, LedBlack);
    delay(10);
    g_encoder.setLEDColor(1, LedBlack);
    delay(10);
    g_encoder.setLEDColor(0, LedBlack);
    delay(10);
  } else if (g_select == 1) {
    g_encoder.setLEDColor(7, (0 <= g_patRtmDur[g_curPat]) ? LedPurple : LedBlack);
    delay(10);
    g_encoder.setLEDColor(6, (1 <= g_patRtmDur[g_curPat]) ? LedPurple : LedBlack);
    delay(10);
    g_encoder.setLEDColor(5, (2 <= g_patRtmDur[g_curPat]) ? LedPurple : LedBlack);
    delay(10);
    g_encoder.setLEDColor(4, (3 <= g_patRtmDur[g_curPat]) ? LedPurple : LedBlack);
    delay(10);
    g_encoder.setLEDColor(3, (4 <= g_patRtmDur[g_curPat]) ? LedPurple : LedBlack);
    delay(10);
    g_encoder.setLEDColor(2, (5 <= g_patRtmDur[g_curPat]) ? LedPurple : LedBlack);
    delay(10);
    g_encoder.setLEDColor(1, (6 <= g_patRtmDur[g_curPat]) ? LedPurple : LedBlack);
    delay(10);
    g_encoder.setLEDColor(0, (7 <= g_patRtmDur[g_curPat]) ? LedPurple : LedBlack);
    delay(10);
  } else {
    g_encoder.setLEDColor(7, (0 <= g_patRtmDur[g_curPat]) ? LedGreen : LedBlack);
    delay(10);
    g_encoder.setLEDColor(6, (1 <= g_patRtmDur[g_curPat]) ? LedGreen : LedBlack);
    delay(10);
    g_encoder.setLEDColor(5, (2 <= g_patRtmDur[g_curPat]) ? LedGreen : LedBlack);
    delay(10);
    g_encoder.setLEDColor(4, (3 <= g_patRtmDur[g_curPat]) ? LedGreen : LedBlack);
    delay(10);
    g_encoder.setLEDColor(3, (4 <= g_patRtmDur[g_curPat]) ? LedGreen : LedBlack);
    delay(10);
    g_encoder.setLEDColor(2, (5 <= g_patRtmDur[g_curPat]) ? LedGreen : LedBlack);
    delay(10);
    g_encoder.setLEDColor(1, (6 <= g_patRtmDur[g_curPat]) ? LedGreen : LedBlack);
    delay(10);
    g_encoder.setLEDColor(0, (7 <= g_patRtmDur[g_curPat]) ? LedGreen : LedBlack);
    delay(10);
  }
}

// ************************************************** **************************************************
// cntlPagePat
// ************************************************** **************************************************
void cntlPagePat() {
  if (checkTouch()) {
    Serial.println("cntlPagePat - Touch");
    if (g_touch.y >= 210) {
      if (g_touch.x < 64) {  // Home Button
        g_page = PagePlay;
        dispPagePlay();
        return;
      } else if (g_touch.x >= 80 && g_touch.x < 144) {  // Return Button
        g_page = PageSeq;
        dispPageSeq();
        return;
      } else if (g_touch.x >= 160 && g_touch.x < 224) {  // Rnd Button
        g_page = PagePatRnd;
        dispPagePatRnd();
        return;
      }
    }

    // Select Button
    if (g_touch.y < 30) {
      if (g_touch.x >= 128 - 16 && g_touch.x < 144 + 16) {
        g_select = 0;
        M5.Display.fillRoundRect(128, 0, 16, 30, 4, Orange);
        M5.Display.fillRoundRect(0, 75, 16, 30, 4, DarkGrey);
        M5.Display.fillRoundRect(0, 150, 16, 30, 4, DarkGrey);

        g_page = PagePat;
        dispPagePat();
        return;
      }
    } else if (g_touch.x < 16 + 16) {
      if (g_touch.y >= 75 - 16 && g_touch.y < 105 + 16) {
        g_select = 1;
        M5.Display.fillRoundRect(128, 0, 16, 30, 4, DarkGrey);
        M5.Display.fillRoundRect(0, 75, 16, 30, 4, Orange);
        M5.Display.fillRoundRect(0, 150, 16, 30, 4, DarkGrey);

        g_page = PagePat;
        dispPagePat();
        return;
      } else if (g_touch.y >= 150 - 16 && g_touch.y < 180 + 16) {
        g_select = 2;
        M5.Display.fillRoundRect(128, 0, 16, 30, 4, DarkGrey);
        M5.Display.fillRoundRect(0, 75, 16, 30, 4, DarkGrey);
        M5.Display.fillRoundRect(0, 150, 16, 30, 4, Orange);

        g_page = PagePat;
        dispPagePat();
        return;
      }
    }
  }

  // Encoder
  M5.Display.setTextColor(White, Black);
  if (g_select == 0) {
    if (checkEncoder(7, 0, 17)) {
      g_curPat = g_curEncoder[7];

      g_page = PagePat;
      dispPagePat();
      return;
    }
    if (checkEncoder(6, 0, 3)) {
      g_patRtmDur[g_curPat] = g_curEncoder[6];

      g_page = PagePat;
      dispPagePat();
      return;
    }
  } else if (g_select == 1) {
    for (int i = 0; i <= g_patRtmDur[g_curPat]; i++) {
      if (checkEncoder(7 - i, 0, 1)) {
        g_patRtm[g_curPat][i] = g_curEncoder[7 - i];
        M5.Display.fillRect(32 + i * 32, 75, 32, 30, Black);
        M5.Display.drawRect(32 + i * 32, 75, 32, 30, Purple);
        M5.Display.drawString(String(g_patRtm[g_curPat][i]), 48 + i * 32, 90);
      }
    }
  } else {
    M5.Display.setFreeFont(&FreeSans9pt7b);
    for (int i = 0; i <= g_patRtmDur[g_curPat]; i++) {
      if (checkEncoder(7 - i, -24, 24)) {
        g_patPitch[g_curPat][i] = g_curEncoder[7 - i];
        M5.Display.fillRect(32 + i * 32, 150, 32, 30, Black);
        M5.Display.drawRect(32 + i * 32, 150, 32, 30, Green);
        M5.Display.drawString(String(g_patPitch[g_curPat][i]), 48 + i * 32, 165);
      }
    }
    M5.Display.setFreeFont(&FreeSans12pt7b);
  }

#ifdef Debug
  // Capture screen
  if (!g_encoder.getButtonStatus(0)) {
    delay(500);
    saveBMP("/disp_pat.bmp");
    delay(500);
  }
#endif
}

// ************************************************** **************************************************
// genPatRnd
// ************************************************** **************************************************
void genPatRnd() {

  // g_patRnd[g_curPat - 18][i] // Seq選択肢の重み
  // g_patRndList[g_curPat - 18][j] // 乱数１６個を格納
  int totalWeight = 0;

  // 総重みを計算
  for (int i = 0; i < 8; i++) {
    totalWeight += g_patRnd[g_curPat - 18][i];
  }

  for (int j = 0; j < 17; j++) {
    int r = random(0, totalWeight);
    int sum = 0;
    int i;

    for (i = 0; i < 8; i++) {
      sum += g_patRnd[g_curPat - 18][i];
      if (r < sum) {
        break;
      }
    }
    if (i == 8) {  // 重みがAll ０の場合
      //i = 0;
      i = random(0, 8);
    }
    g_patRndList[g_curPat - 18][j] = i;
  }

  // g_patRndPitch[g_curPat - 18][i]  // SeqのPitch選択肢の重み
  // g_patRndPitchList[g_curPat - 18][i]  // 乱数１６個を格納
  totalWeight = 0;

  // 総重みを計算
  for (int i = 0; i < 7; i++) {
    totalWeight += g_patRndPitch[g_curPat - 18][i];
  }

  for (int j = 0; j < 17; j++) {
    int r = random(0, totalWeight);
    int sum = 0;
    int i;

    for (i = 0; i < 7; i++) {
      sum += g_patRndPitch[g_curPat - 18][i];
      if (r < sum) {
        break;
      }
    }
    if (i == 7) {  // 重みがAll ０の場合
      //i = 3;
      i = random(0, 7);
    }
    g_patRndPitchList[g_curPat - 18][j] = i;
  }


  Serial.println("genPatRnd " + String((char)(g_curPat + 'a')));
  for (int i = 0; i < 17; i++) {
    Serial.print(" " + String((char)(g_patRndList[g_curPat - 18][i] + 'a')));
  }
  Serial.println();
  for (int i = 0; i < 17; i++) {
    Serial.print(" " + String(g_patRndPitchList[g_curPat - 18][i]));
  }
  Serial.println();
}

// ************************************************** **************************************************
// dispPagePatRnd
// ************************************************** **************************************************
void dispPagePatRnd() {
  Serial.println("dispPagePatRnd");

  M5.Display.clear();
  M5.Display.setTextDatum(MC_DATUM);  // MC_DATUM : 中央、ML_DATUM : 左中央

  // Title
  M5.Display.setTextColor(GREENYELLOW, BLACK);
  M5.Display.drawString(SrcId, 64, 16);

  // Text
  M5.Display.setTextColor(White, Black);

  M5.Display.drawString("Pat", 176, 16);
  M5.Display.drawString("Rnd", 272, 16);

  M5.Display.setFreeFont(&FreeSans9pt7b);
  M5.Display.setTextDatum(TL_DATUM);  // TL_DATUM : 左上

  M5.Display.drawString("Pat Probability", 32, 30 + 4);
  M5.Display.drawString("Pitch Probability", 32, 105 + 4);

  M5.Display.setTextDatum(MC_DATUM);  // MC_DATUM : 中央

  for (int i = 0; i < 8; i++) {
    M5.Display.drawString(String((char)('a' + i)), 48 + i * 32, 60 + 4);
    if (i < 7) {
      M5.Display.drawString(String(i - 3), 48 + i * 32, 135 + 4);
    }
  }

  M5.Display.setFreeFont(&FreeSans12pt7b);

  // Input area
  if (g_curPat < 18) {
    g_curPat = 18;
  }

  M5.Display.drawRect(208, 0, 32, 30, Blue);
  M5.Display.drawString(String((char)(g_curPat + 'a')), 224, 15);

  for (int i = 0; i < 8; i++) {
    M5.Display.drawRect(32 + i * 32, 75, 32, 30, Purple);
    M5.Display.drawString(String(g_patRnd[g_curPat - 18][i]), 48 + i * 32, 91);
  }

  for (int i = 0; i < 7; i++) {
    M5.Display.drawRect(32 + i * 32, 150, 32, 30, Green);
    M5.Display.drawString(String(g_patRndPitch[g_curPat - 18][i]), 48 + i * 32, 166);
  }

  // On/Off Button
  M5.Display.fillRoundRect(128, 0, 16, 30, 4, (g_select == 0) ? Orange : DarkGrey);
  M5.Display.fillRoundRect(0, 75, 16, 30, 4, (g_select == 1) ? Orange : DarkGrey);
  M5.Display.fillRoundRect(0, 150, 16, 30, 4, (g_select == 2) ? Orange : DarkGrey);

  // Encoder
  if (g_select == 0) {
    g_encoder.setEncoderValue(7, g_curPat * 2);
    g_curEncoder[7] = g_curPat;
    delay(10);
  } else if (g_select == 1) {
    for (int i = 0; i < 8; i++) {
      g_encoder.setEncoderValue(7 - i, g_patRnd[g_curPat - 18][i] * 2);
      g_curEncoder[7 - i] = g_patRnd[g_curPat - 18][i];
      delay(10);
    }
  } else {
    for (int i = 0; i < 8; i++) {
      g_encoder.setEncoderValue(7 - i, g_patRndPitch[g_curPat - 18][i] * 2);
      g_curEncoder[7 - i] = g_patRndPitch[g_curPat - 18][i];
      delay(10);
    }
  }

  // Menu Button
  M5.Display.fillRoundRect(0, 210, 64, 30, 4, DarkGrey);
  M5.Display.setTextColor(White, DarkGrey);
  M5.Display.drawString("Home", 32, 226);

  M5.Display.fillRoundRect(80, 210, 64, 30, 4, DarkGrey);
  M5.Display.setTextColor(White, DarkGrey);
  M5.Display.drawString("Ret", 112, 226);

  M5.Display.fillRoundRect(160, 210, 64, 30, 4, DarkGrey);
  M5.Display.setTextColor(White, DarkGrey);
  M5.Display.drawString("Std", 192, 226);

  // LED
  if (g_select == 0) {
    g_encoder.setLEDColor(7, LedCyan);
    delay(10);
    g_encoder.setLEDColor(6, LedBlack);
    delay(10);
    g_encoder.setLEDColor(5, LedBlack);
    delay(10);
    g_encoder.setLEDColor(4, LedBlack);
    delay(10);
    g_encoder.setLEDColor(3, LedBlack);
    delay(10);
    g_encoder.setLEDColor(2, LedBlack);
    delay(10);
    g_encoder.setLEDColor(1, LedBlack);
    delay(10);
    g_encoder.setLEDColor(0, LedBlack);
    delay(10);
  } else if (g_select == 1) {
    g_encoder.setLEDColor(7, LedPurple);
    delay(10);
    g_encoder.setLEDColor(6, LedPurple);
    delay(10);
    g_encoder.setLEDColor(5, LedPurple);
    delay(10);
    g_encoder.setLEDColor(4, LedPurple);
    delay(10);
    g_encoder.setLEDColor(3, LedPurple);
    delay(10);
    g_encoder.setLEDColor(2, LedPurple);
    delay(10);
    g_encoder.setLEDColor(1, LedPurple);
    delay(10);
    g_encoder.setLEDColor(0, LedPurple);
    delay(10);
  } else {
    g_encoder.setLEDColor(7, LedGreen);
    delay(10);
    g_encoder.setLEDColor(6, LedGreen);
    delay(10);
    g_encoder.setLEDColor(5, LedGreen);
    delay(10);
    g_encoder.setLEDColor(4, LedGreen);
    delay(10);
    g_encoder.setLEDColor(3, LedGreen);
    delay(10);
    g_encoder.setLEDColor(2, LedGreen);
    delay(10);
    g_encoder.setLEDColor(1, LedGreen);
    delay(10);
    g_encoder.setLEDColor(0, LedBlack);
    delay(10);
  }
}

// ************************************************** **************************************************
// cntlPagePatRnd
// ************************************************** **************************************************
void cntlPagePatRnd() {
  if (checkTouch()) {
    Serial.println("cntlPagePatRnd - Touch");
    if (g_touch.y >= 210) {
      if (g_touch.x < 64) {  // Home Button
        genPatRnd();
        g_page = PagePlay;
        dispPagePlay();
        return;
      } else if (g_touch.x >= 80 && g_touch.x < 144) {  // Return Button
        genPatRnd();
        g_page = PageSeq;
        dispPageSeq();
        return;
      } else if (g_touch.x >= 160 && g_touch.x < 224) {  // Std Button
        genPatRnd();
        g_page = PagePat;
        dispPagePat();
        return;
      }
    }

    // Select Button
    if (g_touch.y < 30) {
      if (g_touch.x >= 128 - 16 && g_touch.x < 144 + 16) {
        g_select = 0;
        M5.Display.fillRoundRect(128, 0, 16, 30, 4, Orange);
        M5.Display.fillRoundRect(0, 75, 16, 30, 4, DarkGrey);
        M5.Display.fillRoundRect(0, 150, 16, 30, 4, DarkGrey);

        g_page = PagePatRnd;
        dispPagePatRnd();
        return;
      }
    } else if (g_touch.x < 16 + 16) {
      if (g_touch.y >= 75 - 16 && g_touch.y < 105 + 16) {
        g_select = 1;
        M5.Display.fillRoundRect(128, 0, 16, 30, 4, DarkGrey);
        M5.Display.fillRoundRect(0, 75, 16, 30, 4, Orange);
        M5.Display.fillRoundRect(0, 150, 16, 30, 4, DarkGrey);

        g_page = PagePatRnd;
        dispPagePatRnd();
        return;
      } else if (g_touch.y >= 150 - 16 && g_touch.y < 180 + 16) {
        g_select = 2;
        M5.Display.fillRoundRect(128, 0, 16, 30, 4, DarkGrey);
        M5.Display.fillRoundRect(0, 75, 16, 30, 4, DarkGrey);
        M5.Display.fillRoundRect(0, 150, 16, 30, 4, Orange);

        g_page = PagePatRnd;
        dispPagePatRnd();
        return;
      }
    }
  }

  // Encoder
  M5.Display.setTextColor(White, Black);
  if (g_select == 0) {
    if (checkEncoder(7, 18, 25, true)) {
      genPatRnd();
      g_curPat = g_curEncoder[7];
      g_page = PagePatRnd;
      dispPagePatRnd();
      return;
    }
  } else if (g_select == 1) {
    for (int i = 0; i < 8; i++) {
      if (checkEncoder(7 - i, 0, 9)) {
        g_patRnd[g_curPat - 18][i] = g_curEncoder[7 - i];
        M5.Display.fillRect(32 + i * 32, 75, 32, 30, Black);
        M5.Display.drawRect(32 + i * 32, 75, 32, 30, Purple);
        M5.Display.drawString(String(g_patRnd[g_curPat - 18][i]), 48 + i * 32, 90);
      }
    }
  } else {
    for (int i = 0; i < 7; i++) {
      if (checkEncoder(7 - i, 0, 9)) {
        g_patRndPitch[g_curPat - 18][i] = g_curEncoder[7 - i];
        M5.Display.fillRect(32 + i * 32, 150, 32, 30, Black);
        M5.Display.drawRect(32 + i * 32, 150, 32, 30, Green);
        M5.Display.drawString(String(g_patRndPitch[g_curPat - 18][i]), 48 + i * 32, 165);
      }
    }
  }

#ifdef Debug
  // Capture screen
  if (!g_encoder.getButtonStatus(0)) {
    delay(500);
    saveBMP("/disp_patrnd.bmp");
    delay(500);
  }
#endif
}

// ************************************************** **************************************************
// dispPageFile
// ************************************************** **************************************************
void dispPageFile() {
  Serial.println("dispPageFile");

  M5.Display.clear();
  M5.Display.setTextDatum(MC_DATUM);  // MC_DATUM : 中央、ML_DATUM : 左中央

  // Title
  M5.Display.setTextColor(GREENYELLOW, Black);
  M5.Display.drawString(SrcId, 64, 16);

  M5.Display.fillRect(192, 0, 64, 30, DarkPurple);
  M5.Display.setTextColor(White, DarkPurple);
  M5.Display.drawString("File", 224, 16);

  // Load Button
  M5.Display.fillRoundRect(64, 60, 64, 30, 4, DarkBlue);
  M5.Display.setTextColor(White, DarkBlue);
  M5.Display.drawString("Load", 96, 76);

  // Save Button
  M5.Display.fillRoundRect(64, 150, 64, 30, 4, DarkBlue);
  M5.Display.setTextColor(White, DarkBlue);
  M5.Display.drawString("Save", 96, 166);

  // Rename Button
  M5.Display.fillRoundRect(192, 60, 64, 30, 4, DarkBlue);
  M5.Display.setTextColor(White, DarkBlue);
  M5.Display.drawString("Ren", 224, 76);

  // Delete Button
  M5.Display.fillRoundRect(192, 150, 64, 30, 4, DarkBlue);
  M5.Display.setTextColor(White, DarkBlue);
  M5.Display.drawString("Del", 224, 166);

  // Menu Button
  M5.Display.fillRoundRect(0, 210, 64, 30, 4, DARKGREY);
  M5.Display.setTextColor(White, DARKGREY);
  M5.Display.drawString("Ret", 32, 226);

  // LED
  g_encoder.setLEDColor(7, LedBlack);
  delay(10);
  g_encoder.setLEDColor(6, LedBlack);
  delay(10);
  g_encoder.setLEDColor(5, LedBlack);
  delay(10);
  g_encoder.setLEDColor(4, LedBlack);
  delay(10);
  g_encoder.setLEDColor(3, LedBlack);
  delay(10);
  g_encoder.setLEDColor(2, LedBlack);
  delay(10);
  g_encoder.setLEDColor(1, LedBlack);
  delay(10);
  g_encoder.setLEDColor(0, LedBlack);
  delay(10);
}

// ************************************************** **************************************************
// cntlPageFile
// ************************************************** **************************************************
void cntlPageFile() {
  if (checkTouch()) {
    Serial.println("cntlPageFile - Touch");
    if (g_touch.y >= 210) {
      if (g_touch.x < 64) {  // Return Button
        g_page = PagePlay;
        dispPagePlay();
        return;
      }
    }

    if (g_touch.y >= 50 && g_touch.y < 100) {
      if (g_touch.x >= 64 && g_touch.x < 128) {  // Load Button
        g_page = PageLoad;
        dispPageLoad();
        return;
      } else if (g_touch.x >= 192 && g_touch.x < 256) {  // Rename Button
        g_page = PageRename;
        dispPageRename();
        return;
      }
    }

    if (g_touch.y >= 140 && g_touch.y < 190) {
      if (g_touch.x >= 64 && g_touch.x < 128) {  // Save Button
        g_page = PageSave;
        dispPageSave();
        return;
      } else if (g_touch.x >= 192 && g_touch.x < 256) {  // Delete Button
        g_page = PageDelete;
        dispPageDelete();
        return;
      }
    }
  }

#ifdef Debug
  // Capture screen
  if (!g_encoder.getButtonStatus(0)) {
    delay(500);
    saveBMP("/disp_file.bmp");
    delay(500);
  }
#endif
}

// ************************************************** **************************************************
// dispPageLoad
// ************************************************** **************************************************
void dispPageLoad() {
  Serial.println("dispPageLoad");

  M5.Display.clear();
  M5.Display.setTextDatum(MC_DATUM);  // MC_DATUM : 中央、ML_DATUM : 左中央

  // Title
  M5.Display.setTextColor(GREENYELLOW, Black);
  M5.Display.drawString(SrcId, 64, 16);

  M5.Display.fillRect(192, 0, 64, 30, DarkPurple);
  M5.Display.setTextColor(White, DarkPurple);
  M5.Display.drawString("File", 224, 16);

  M5.Display.setTextColor(White, Black);
  M5.Display.drawString("Load", 288, 16);

  // Menu Button
  M5.Display.fillRoundRect(0, 210, 64, 30, 4, DARKGREY);
  M5.Display.setTextColor(White, DARKGREY);
  M5.Display.drawString("Ret", 32, 226);

  M5.Display.fillRoundRect(128, 210, 64, 30, 4, DarkBlue);
  M5.Display.setTextColor(White, DarkBlue);
  M5.Display.drawString("Cncl", 160, 226);

  M5.Display.fillRoundRect(256, 210, 64, 30, 4, DarkBlue);
  M5.Display.setTextColor(White, DarkBlue);
  M5.Display.drawString("OK", 288, 226);

  // SDカードのファイル名を読み込んで表示
  getFileList();
  if (g_fileCount == 1) {
    g_page = PagePlay;
    dispPagePlay();
    return;
  }
  g_fileList[g_fileCount + 1] = "";
  g_fileList[g_fileCount + 2] = "";

  // File Name
  M5.Display.setTextDatum(ML_DATUM);  // MC_DATUM : 中央、ML_DATUM : 左中央
  M5.Display.fillRect(64, 105, 160, 30, DarkYellow);
  for (int i = 0; i < 5 && i <= g_fileCount; i++) {
    if (i == 2) {
      M5.Display.setTextColor(WHITE, DarkYellow);
    } else {
      M5.Display.setTextColor(WHITE, BLACK);
    }
    M5.Display.drawString(g_fileList[i], 96, i * 30 + 61);
  }
  g_fileDisp = 0;
  g_encoder.setEncoderValue(7, 0);
  g_curEncoder[7] = 0;
  g_curFile = g_fileList[2];

  // LED
  g_encoder.setLEDColor(7, LedCyan);
  delay(10);
  g_encoder.setLEDColor(6, LedBlack);
  delay(10);
  g_encoder.setLEDColor(5, LedBlack);
  delay(10);
  g_encoder.setLEDColor(4, LedBlack);
  delay(10);
  g_encoder.setLEDColor(3, LedBlack);
  delay(10);
  g_encoder.setLEDColor(2, LedBlack);
  delay(10);
  g_encoder.setLEDColor(1, LedBlack);
  delay(10);
  g_encoder.setLEDColor(0, LedBlack);
  delay(10);
}

// ************************************************** **************************************************
// cntlPageLoad
// ************************************************** **************************************************
void cntlPageLoad() {
  if (checkTouch()) {
    Serial.println("cntlPageLoad - Touch");
    if (g_touch.y >= 210) {
      if (g_touch.x < 64) {  // Return Button
        g_page = PageFile;
        dispPageFile();
        return;
      } else if (g_touch.x >= 128 && g_touch.x < 192) {  // Cancel Button
        g_page = PageFile;
        dispPageFile();
        return;
      } else if (g_touch.x >= 256) {  // OK Button

        // ファイルのLoad
        Serial.println("cntlPageLoad - OK File:" + g_curFile);
        if (loadJson()) {
          // 正常時
          g_loadedFile = g_curFile;
          g_page = PagePlay;
          dispPagePlay();
        } else {
          // エラー時
          g_page = PagePlay;
          dispPagePlay();
          M5.Display.setTextColor(Red, Black);
          M5.Display.drawString("Err", 160, 16);
        }
        return;
      }
    }
  }

  // ファイルスクロール
  scrollList();

#ifdef Debug
  // Capture screen
  if (!g_encoder.getButtonStatus(0)) {
    delay(500);
    saveBMP("/disp_load.bmp");
    delay(500);
  }
#endif
}

// ************************************************** **************************************************
// dispPageSave
// ************************************************** **************************************************
void dispPageSave() {
  Serial.println("dispPageSave");

  M5.Display.clear();
  M5.Display.setTextDatum(MC_DATUM);  // MC_DATUM : 中央、ML_DATUM : 左中央

  // Title
  M5.Display.setTextColor(GREENYELLOW, Black);
  M5.Display.drawString(SrcId, 64, 16);

  M5.Display.fillRect(192, 0, 64, 30, DarkPurple);
  M5.Display.setTextColor(White, DarkPurple);
  M5.Display.drawString("File", 224, 16);

  M5.Display.setTextColor(White, Black);
  M5.Display.drawString("Save", 288, 16);

  // Menu Button
  M5.Display.fillRoundRect(0, 210, 64, 30, 4, DARKGREY);
  M5.Display.setTextColor(White, DARKGREY);
  M5.Display.drawString("Ret", 32, 226);

  M5.Display.fillRoundRect(128, 210, 64, 30, 4, DarkBlue);
  M5.Display.setTextColor(White, DarkBlue);
  M5.Display.drawString("Cncl", 160, 226);

  M5.Display.fillRoundRect(256, 210, 64, 30, 4, DarkBlue);
  M5.Display.setTextColor(White, DarkBlue);
  M5.Display.drawString("OK", 288, 226);

  // SDカードのファイル名を読み込んで表示
  getFileList();
  g_fileList[++g_fileCount] = "<New>";
  g_fileList[g_fileCount + 1] = "";
  g_fileList[g_fileCount + 2] = "";

  int loadedPos = 0;  // ********** NEW ********** NEW ********** NEW **********
  for (int i = 2; i < g_fileCount; i++) {
    if (g_fileList[i] == g_loadedFile) {
      loadedPos = i - 2;
      break;
    }
  }

  g_fileDisp = loadedPos;
  g_encoder.setEncoderValue(7, g_fileDisp * 2);
  g_curEncoder[7] = g_fileDisp;

  // File Name
  M5.Display.setTextDatum(ML_DATUM);  // MC_DATUM : 中央、ML_DATUM : 左中央
  M5.Display.fillRect(64, 105, 160, 30, DarkYellow);
  for (int i = 0; i < 5 && i <= g_fileCount; i++) {
    if (i == 2) {
      M5.Display.setTextColor(WHITE, DarkYellow);
    } else {
      M5.Display.setTextColor(WHITE, BLACK);
    }
    M5.Display.drawString(g_fileList[i + g_fileDisp], 96, i * 30 + 61);
  }

  g_curFile = g_fileList[g_fileDisp + 2];

  // LED
  g_encoder.setLEDColor(7, LedCyan);
  delay(10);
  g_encoder.setLEDColor(6, LedBlack);
  delay(10);
  g_encoder.setLEDColor(5, LedBlack);
  delay(10);
  g_encoder.setLEDColor(4, LedBlack);
  delay(10);
  g_encoder.setLEDColor(3, LedBlack);
  delay(10);
  g_encoder.setLEDColor(2, LedBlack);
  delay(10);
  g_encoder.setLEDColor(1, LedBlack);
  delay(10);
  g_encoder.setLEDColor(0, LedBlack);
  delay(10);
}

// ************************************************** **************************************************
// cntlPageSave
// ************************************************** **************************************************
void cntlPageSave() {
  if (checkTouch()) {
    Serial.println("cntlPageSave - Touch");
    if (g_touch.y >= 210) {
      if (g_touch.x < 64) {  // Return Button
        g_page = PageFile;
        dispPageFile();
        return;
      } else if (g_touch.x >= 128 && g_touch.x < 192) {  // Cancel Button
        g_page = PageFile;
        dispPageFile();
        return;
      } else if (g_touch.x >= 256) {  // OK Button

        // ファイルのSave

        if (g_curFile == "<New>") {
          // dispPageInputでファイル名を入力
          g_caller = PageSave;
          g_page = PageInput;
          dispPageInput();
          Serial.println("cntlPageSave - OK Input:" + g_curFile);
          return;
        } else {
          Serial.println("cntlPageSave - OK Save:" + g_curFile);
          // g_curFileのファイルにJsonを保存
          saveJson();
          g_loadedFile = g_curFile;

          g_page = PagePlay;
          dispPagePlay();
          return;
        }
      }
    }
  }

  // ファイルスクロール
  scrollList();

#ifdef Debug
  // Capture screen
  if (!g_encoder.getButtonStatus(0)) {
    delay(500);
    saveBMP("/disp_save.bmp");
    delay(500);
  }
#endif
}

// ************************************************** **************************************************
// dispPageRename
// ************************************************** **************************************************
void dispPageRename() {
  Serial.println("dispPageRename");

  M5.Display.clear();
  M5.Display.setTextDatum(MC_DATUM);  // MC_DATUM : 中央、ML_DATUM : 左中央

  // Title
  M5.Display.setTextColor(GREENYELLOW, Black);
  M5.Display.drawString(SrcId, 64, 16);

  M5.Display.fillRect(192, 0, 64, 30, DarkPurple);
  M5.Display.setTextColor(White, DarkPurple);
  M5.Display.drawString("File", 224, 16);

  M5.Display.setTextColor(White, Black);
  M5.Display.drawString("Ren", 288, 16);

  // Menu Button
  M5.Display.fillRoundRect(0, 210, 64, 30, 4, DARKGREY);
  M5.Display.setTextColor(White, DARKGREY);
  M5.Display.drawString("Ret", 32, 226);

  M5.Display.fillRoundRect(128, 210, 64, 30, 4, DarkBlue);
  M5.Display.setTextColor(White, DarkBlue);
  M5.Display.drawString("Cncl", 160, 226);

  M5.Display.fillRoundRect(256, 210, 64, 30, 4, DarkBlue);
  M5.Display.setTextColor(White, DarkBlue);
  M5.Display.drawString("OK", 288, 226);

  // SDカードのファイル名を読み込んで表示
  getFileList();
  g_fileList[g_fileCount + 1] = "";
  g_fileList[g_fileCount + 2] = "";

  // File Name
  M5.Display.setTextDatum(ML_DATUM);  // MC_DATUM : 中央、ML_DATUM : 左中央
  M5.Display.fillRect(64, 105, 160, 30, DarkYellow);
  for (int i = 0; i < 5 && i <= g_fileCount; i++) {
    if (i == 2) {
      M5.Display.setTextColor(WHITE, DarkYellow);
    } else {
      M5.Display.setTextColor(WHITE, BLACK);
    }
    M5.Display.drawString(g_fileList[i], 96, i * 30 + 61);
  }
  g_fileDisp = 0;
  g_encoder.setEncoderValue(7, 0);
  g_curEncoder[7] = 0;
  g_curFile = g_fileList[2];

  // LED
  g_encoder.setLEDColor(7, LedCyan);
  delay(10);
  g_encoder.setLEDColor(6, LedBlack);
  delay(10);
  g_encoder.setLEDColor(5, LedBlack);
  delay(10);
  g_encoder.setLEDColor(4, LedBlack);
  delay(10);
  g_encoder.setLEDColor(3, LedBlack);
  delay(10);
  g_encoder.setLEDColor(2, LedBlack);
  delay(10);
  g_encoder.setLEDColor(1, LedBlack);
  delay(10);
  g_encoder.setLEDColor(0, LedBlack);
  delay(10);
}

// ************************************************** **************************************************
// cntlPageRename
// ************************************************** **************************************************
void cntlPageRename() {
  if (checkTouch()) {
    Serial.println("cntlPageRename - Touch");
    if (g_touch.y >= 210) {
      if (g_touch.x < 64) {  // Return Button
        g_page = PageFile;
        dispPageFile();
        return;
      } else if (g_touch.x >= 128 && g_touch.x < 192) {  // Cancel Button
        g_page = PageFile;
        dispPageFile();
        return;
      } else if (g_touch.x >= 256) {  // OK Button

        // ファイルのRename
        Serial.println("cntlPageRename - OK Input:" + g_curFile);

        g_caller = PageRename;
        g_page = PageInput;
        dispPageInput();
        return;
      }
    }
  }

  // ファイルスクロール
  scrollList();

#ifdef Debug
  // Capture screen
  if (!g_encoder.getButtonStatus(0)) {
    delay(500);
    saveBMP("/disp_rename.bmp");
    delay(500);
  }
#endif
}

// ************************************************** **************************************************
// dispPageDelete
// ************************************************** **************************************************
void dispPageDelete() {
  Serial.println("dispPageDelete");

  M5.Display.clear();
  M5.Display.setTextDatum(MC_DATUM);  // MC_DATUM : 中央、ML_DATUM : 左中央

  // Title
  M5.Display.setTextColor(GREENYELLOW, Black);
  M5.Display.drawString(SrcId, 64, 16);

  M5.Display.fillRect(192, 0, 64, 30, DarkPurple);
  M5.Display.setTextColor(White, DarkPurple);
  M5.Display.drawString("File", 224, 16);

  M5.Display.setTextColor(White, Black);
  M5.Display.drawString("Del", 288, 16);

  // Menu Button
  M5.Display.fillRoundRect(0, 210, 64, 30, 4, DARKGREY);
  M5.Display.setTextColor(White, DARKGREY);
  M5.Display.drawString("Ret", 32, 226);

  M5.Display.fillRoundRect(128, 210, 64, 30, 4, DarkBlue);
  M5.Display.setTextColor(White, DarkBlue);
  M5.Display.drawString("Cncl", 160, 226);

  M5.Display.fillRoundRect(256, 210, 64, 30, 4, DarkBlue);
  M5.Display.setTextColor(White, DarkBlue);
  M5.Display.drawString("OK", 288, 226);

  // SDカードのファイル名を読み込んで表示
  getFileList();
  g_fileList[g_fileCount + 1] = "";
  g_fileList[g_fileCount + 2] = "";

  // File Name
  M5.Display.setTextDatum(ML_DATUM);  // MC_DATUM : 中央、ML_DATUM : 左中央
  M5.Display.fillRect(64, 105, 160, 30, DarkYellow);
  for (int i = 0; i < 5 && i <= g_fileCount; i++) {
    if (i == 2) {
      M5.Display.setTextColor(WHITE, DarkYellow);
    } else {
      M5.Display.setTextColor(WHITE, BLACK);
    }
    M5.Display.drawString(g_fileList[i], 96, i * 30 + 61);
  }
  g_fileDisp = 0;
  g_encoder.setEncoderValue(7, 0);
  g_curEncoder[7] = 0;
  g_curFile = g_fileList[2];

  // LED
  g_encoder.setLEDColor(7, LedCyan);
  delay(10);
  g_encoder.setLEDColor(6, LedBlack);
  delay(10);
  g_encoder.setLEDColor(5, LedBlack);
  delay(10);
  g_encoder.setLEDColor(4, LedBlack);
  delay(10);
  g_encoder.setLEDColor(3, LedBlack);
  delay(10);
  g_encoder.setLEDColor(2, LedBlack);
  delay(10);
  g_encoder.setLEDColor(1, LedBlack);
  delay(10);
  g_encoder.setLEDColor(0, LedBlack);
  delay(10);
}

// ************************************************** **************************************************
// cntlPageDelete
// ************************************************** **************************************************
void cntlPageDelete() {
  if (checkTouch()) {
    Serial.println("cntlPageDelete - Touch");
    if (g_touch.y >= 210) {
      if (g_touch.x < 64) {  // Return Button
        g_page = PageFile;
        dispPageFile();
        return;
      } else if (g_touch.x >= 128 && g_touch.x < 192) {  // Cancel Button
        g_page = PageFile;
        dispPageFile();
        return;
      } else if (g_touch.x >= 256) {  // OK Button

        // ファイルのDelete
        Serial.println("cntlPageDelete - OK File:" + g_curFile);

        if (SD.remove("/" + g_curFile + ".json")) {
          Serial.println("ファイルを削除しました");
        } else {
          Serial.println("ファイルの削除に失敗しました");
        }

        g_page = PagePlay;
        dispPagePlay();
        return;
      }
    }
  }

  // ファイルスクロール
  scrollList();

#ifdef Debug
  // Capture screen
  if (!g_encoder.getButtonStatus(0)) {
    delay(500);
    saveBMP("/disp_delete.bmp");
    delay(500);
  }
#endif
}

// ************************************************** **************************************************
// dispPageInput
// ************************************************** **************************************************
void dispPageInput() {
  Serial.println("dispPageInput");

  M5.Display.clear();
  M5.Display.setTextDatum(MC_DATUM);  // MC_DATUM : 中央、ML_DATUM : 左中央

  // Title
  M5.Display.setTextColor(GREENYELLOW, Black);
  M5.Display.drawString(SrcId, 64, 16);

  M5.Display.fillRect(192, 0, 64, 30, DarkPurple);
  M5.Display.setTextColor(White, DarkPurple);
  M5.Display.drawString("File", 224, 16);

  M5.Display.setTextColor(White, Black);
  switch (g_caller) {
    case PageSave:
      g_oldFile = "a       ";
      M5.Display.drawString("Save", 288, 16);
      break;

    case PageRename:
      g_oldFile = g_curFile;
      M5.Display.drawString("Ren", 288, 16);
      break;

    default:
      break;
  }

  // Menu Button
  M5.Display.fillRoundRect(0, 210, 64, 30, 4, DARKGREY);
  M5.Display.setTextColor(White, DARKGREY);
  M5.Display.drawString("Ret", 32, 226);

  M5.Display.fillRoundRect(128, 210, 64, 30, 4, DarkBlue);
  M5.Display.setTextColor(White, DarkBlue);
  M5.Display.drawString("Cncl", 160, 226);

  M5.Display.fillRoundRect(256, 210, 64, 30, 4, DarkBlue);
  M5.Display.setTextColor(White, DarkBlue);
  M5.Display.drawString("OK", 288, 226);

  // Input Area
  M5.Display.fillRect(64, 105, 192, 30, Blue);
  M5.Display.fillRect(64, 105, 24, 30, White);
  M5.Display.setTextColor(Black, White);

  for (int i = 0; i < 8; i++) {
    M5.Display.drawString(String(g_oldFile.charAt(i)), 76 + i * 24, 120);
    M5.Display.setTextColor(White, Blue);
  }

  for (int i = 0; i < 8; i++) {
    if (i < g_oldFile.length()) {
      for (int j = 0; j < strlen(CharSet); j++) {
        if (g_oldFile.charAt(i) == CharSet[j]) {
          g_nameData[i] = j;
          break;
        }
      }
    } else {
      g_nameData[i] = 64;
    }
  }

  g_namePos = 0;
  g_nameChar = 0;
  g_encoder.setEncoderValue(7, 0);
  g_encoder.setEncoderValue(6, g_nameData[0] * 2);

  // LED
  g_encoder.setLEDColor(7, LedCyan);
  delay(10);
  g_encoder.setLEDColor(6, LedWhite);
  delay(10);
  g_encoder.setLEDColor(5, LedBlack);
  delay(10);
  g_encoder.setLEDColor(4, LedBlack);
  delay(10);
  g_encoder.setLEDColor(3, LedBlack);
  delay(10);
  g_encoder.setLEDColor(2, LedBlack);
  delay(10);
  g_encoder.setLEDColor(1, LedBlack);
  delay(10);
  g_encoder.setLEDColor(0, LedBlack);
  delay(10);
}

// ************************************************** **************************************************
// cntlPageInput
// ************************************************** **************************************************
void cntlPageInput() {
  if (checkTouch()) {
    Serial.println("cntlPageInput - Touch");
    if (g_touch.y >= 210) {
      if (g_touch.x < 64) {  // Return Button
        g_page = PageFile;
        dispPageFile();
        return;
      } else if (g_touch.x >= 128 && g_touch.x < 192) {  // Cancel Button
        switch (g_caller) {
          case PageSave:
            g_page = PageSave;
            dispPageSave();
            break;

          case PageRename:
            g_page = PageRename;
            dispPageRename();
            break;

          default:
            break;
        }

      } else if (g_touch.x >= 256) {  // OK Button

        g_curFile = "";
        for (int i = 0; i < 8; i++) {
          g_curFile += String(CharSet[g_nameData[i]]);
        }
        g_curFile.trim();

        switch (g_caller) {
          case PageSave:
            Serial.println("cntlPageInput - Save OK as:" + g_curFile);
            // Save Json
            saveJson();
            g_loadedFile = g_curFile;
            break;

          case PageRename:
            Serial.println("cntlPageInput - Rename OK From:" + g_oldFile + " To:" + g_curFile);
            // Rename File
            if (SD.rename("/" + g_oldFile + ".json", "/" + g_curFile + ".json")) {
              Serial.println("ファイル名を変更しました");
            } else {
              Serial.println("リネームに失敗しました");
            }
            break;

          default:
            break;
        }

        g_page = PagePlay;
        dispPagePlay();
        return;
      }
    }
  }

  // Position
  if (checkEncoder(7, 0, 7)) {
    M5.Display.setTextDatum(MC_DATUM);  // MC_DATUM : 中央、ML_DATUM : 左中央

    M5.Display.fillRect(64 + g_namePos * 24, 105, 24, 30, Blue);
    M5.Display.setTextColor(White, Blue);
    M5.Display.drawString(String(CharSet[g_nameData[g_namePos]]), 76 + g_namePos * 24, 120);

    g_namePos = g_curEncoder[7];
    M5.Display.fillRect(64 + g_namePos * 24, 105, 24, 30, White);
    M5.Display.setTextColor(Black, White);
    M5.Display.drawString(String(CharSet[g_nameData[g_namePos]]), 76 + g_namePos * 24, 120);

    g_encoder.setEncoderValue(6, g_nameData[g_namePos] * 2);
  }

  // Character
  if (checkEncoder(6, 0, 64, true)) {
    M5.Display.setTextDatum(MC_DATUM);  // MC_DATUM : 中央、ML_DATUM : 左中央

    g_nameChar = g_curEncoder[6];
    g_nameData[g_namePos] = g_nameChar;
    M5.Display.fillRect(64 + g_namePos * 24, 105, 24, 30, White);
    M5.Display.setTextColor(Black, White);
    M5.Display.drawString(String(CharSet[g_nameData[g_namePos]]), 76 + g_namePos * 24, 120);
  }

#ifdef Debug
  // Capture screen
  if (!g_encoder.getButtonStatus(0)) {
    delay(500);
    saveBMP("/disp_input.bmp");
    delay(500);
  }
#endif
}

// ************************************************** **************************************************
// dispPageSet1
// ************************************************** **************************************************
void dispPageSet1() {
  Serial.println("dispPageSet1");

  M5.Display.clear();
  M5.Display.setTextDatum(MC_DATUM);  // MC_DATUM : 中央、ML_DATUM : 左中央

  // Title
  M5.Display.setTextColor(GREENYELLOW, BLACK);
  M5.Display.drawString(SrcId, 64, 16);

  // Text
  M5.Display.setTextColor(White, Black);

  M5.Display.drawString("Chord", 64, 60);
  M5.Display.drawString("Dur", 64, 90);
  M5.Display.drawString("Level", 64, 120);
  M5.Display.drawString("Ch", 64, 150);
  M5.Display.drawString("Pc", 64, 180);

  M5.Display.setFreeFont(&FreeSans9pt7b);

  M5.Display.drawString("Osc1", 128, 30);
  M5.Display.drawString("Osc2", 176, 30);
  M5.Display.drawString("Osc3", 224, 30);
  //M5.Display.drawString("Osc4", 272, 30);

  // Input area
  for (int i = 0; i < 4; i++) {
    if (i != 0) {
      M5.Display.drawRect(112 + i * 48, 45, 32, 30, DarkCyan);
      M5.Display.drawRect(112 + i * 48, 75, 32, 30, Blue);

      M5.Display.drawString(String(g_setChord[i]), 128 + i * 48, 60);
      M5.Display.drawString(String(NoteDurName[g_setDur[i] + 4]), 128 + i * 48, 90);
    }
    M5.Display.drawRect(112 + i * 48, 105, 32, 30, Purple);
    M5.Display.drawRect(112 + i * 48, 135, 32, 30, Red);
    M5.Display.drawRect(112 + i * 48, 165, 32, 30, Green);


    M5.Display.drawString(String(g_setLevel[i]), 128 + i * 48, 120);
    M5.Display.drawString(String(g_setCh[i] + 1), 128 + i * 48, 150);
    M5.Display.drawString(String(g_setPc[i] + 1), 128 + i * 48, 180);
    if (i % 2 == 0) {
      M5.Display.drawString(String(GmInstruments[g_setPc[i]]), 128 + i * 48, 210);
    } else {
      M5.Display.drawString(String(GmInstruments[g_setPc[i]]), 128 + i * 48, 227);
    }
  }

  M5.Display.setFreeFont(&FreeSans12pt7b);

  // On/Off Button
  M5.Display.fillRoundRect(0, 45, 16, 30, 4, (g_select == 0) ? Orange : DarkGrey);
  M5.Display.fillRoundRect(0, 105, 16, 30, 4, (g_select == 1) ? Orange : DarkGrey);
  M5.Display.fillRoundRect(0, 165, 16, 30, 4, (g_select == 2) ? Orange : DarkGrey);

  // Encoder
  if (g_select == 0) {
    for (int i = 0; i < 4; i++) {
      g_encoder.setEncoderValue(7 - i, g_setChord[i] * 2);
      g_curEncoder[7 - i] = g_setChord[i];
      g_encoder.setEncoderValue(3 - i, g_setDur[i] * 2);
      g_curEncoder[3 - i] = g_setDur[i];
      delay(10);
    }
  } else if (g_select == 1) {
    for (int i = 0; i < 4; i++) {
      g_encoder.setEncoderValue(7 - i, g_setLevel[i] * 2);
      g_curEncoder[7 - i] = g_setLevel[i];
      g_encoder.setEncoderValue(3 - i, g_setCh[i] * 2);
      g_curEncoder[3 - i] = g_setCh[i];
      delay(10);
    }
  } else {
    for (int i = 0; i < 4; i++) {
      g_encoder.setEncoderValue(7 - i, g_setPc[i] * 2);
      g_curEncoder[7 - i] = g_setPc[i];
      delay(10);
    }
  }

  // Menu Button
  M5.Display.fillRoundRect(0, 210, 64, 30, 4, DarkGrey);
  M5.Display.setTextColor(White, DarkGrey);
  M5.Display.drawString("Home", 32, 226);

  M5.Display.fillRoundRect(256, 0, 64, 30, 4, DarkGrey);
  M5.Display.drawString("Tune", 288, 15);

  // LED
  if (g_select == 0) {
    g_encoder.setLEDColor(7, LedBlack);
    delay(10);
    g_encoder.setLEDColor(6, LedCyan);
    delay(10);
    g_encoder.setLEDColor(5, LedCyan);
    delay(10);
    g_encoder.setLEDColor(4, LedCyan);
    delay(10);
    g_encoder.setLEDColor(3, LedBlack);
    delay(10);
    g_encoder.setLEDColor(2, LedBlue);
    delay(10);
    g_encoder.setLEDColor(1, LedBlue);
    delay(10);
    g_encoder.setLEDColor(0, LedBlue);
    delay(10);
  } else if (g_select == 1) {
    g_encoder.setLEDColor(7, LedPurple);
    delay(10);
    g_encoder.setLEDColor(6, LedPurple);
    delay(10);
    g_encoder.setLEDColor(5, LedPurple);
    delay(10);
    g_encoder.setLEDColor(4, LedPurple);
    delay(10);
    g_encoder.setLEDColor(3, LedRed);
    delay(10);
    g_encoder.setLEDColor(2, LedRed);
    delay(10);
    g_encoder.setLEDColor(1, LedRed);
    delay(10);
    g_encoder.setLEDColor(0, LedRed);
    delay(10);
  } else {
    g_encoder.setLEDColor(7, LedGreen);
    delay(10);
    g_encoder.setLEDColor(6, LedGreen);
    delay(10);
    g_encoder.setLEDColor(5, LedGreen);
    delay(10);
    g_encoder.setLEDColor(4, LedGreen);
    delay(10);
    g_encoder.setLEDColor(3, LedBlack);
    delay(10);
    g_encoder.setLEDColor(2, LedBlack);
    delay(10);
    g_encoder.setLEDColor(1, LedBlack);
    delay(10);
    g_encoder.setLEDColor(0, LedBlack);
    delay(10);
  }

  for (int i = 0; i < 8; i++) {
    if (g_select == 0) {
      switch (i) {
        case 7:
        case 3:
          g_encoder.setLEDColor(i, LedBlack);
          break;
        case 6:
        case 5:
        case 4:
          g_encoder.setLEDColor(i, LedCyan);
          break;

        case 2:
        case 1:
        case 0:
          g_encoder.setLEDColor(i, LedBlue);
          break;
      }
    } else if (g_select == 1) {
      switch (i) {
        case 7:
        case 6:
        case 5:
        case 4:
          g_encoder.setLEDColor(i, LedPurple);
          break;
        case 3:
        case 2:
        case 1:
        case 0:
          g_encoder.setLEDColor(i, LedRed);
          break;
      }
    } else {
      switch (i) {
        case 7:
        case 6:
        case 5:
        case 4:
          g_encoder.setLEDColor(i, LedGreen);
          break;
        case 3:
        case 2:
        case 1:
        case 0:
          g_encoder.setLEDColor(i, LedBlack);
          break;
      }
    }
    delay(10);
  }
}

// ************************************************** **************************************************
// cntlPageSet1
// ************************************************** **************************************************
void cntlPageSet1() {
  if (checkTouch()) {
    Serial.println("cntlPageSet1 - Touch");
    if (g_touch.y >= 210) {
      if (g_touch.x < 64) {  // Home Button
        g_page = PagePlay;
        dispPagePlay();
        return;
      }
    } else if (g_touch.y < 30) {
      if (g_touch.x >= 256) {  // Tune Button
        g_page = PageSet2;
        dispPageSet2();
        return;
      }
    }

    // Select Button
    if (g_touch.x < 16 + 16) {
      if (g_touch.y >= 45 - 15 && g_touch.y < 75 + 15) {
        g_select = 0;
        M5.Display.fillRoundRect(0, 45, 16, 30, 4, Orange);
        M5.Display.fillRoundRect(0, 105, 16, 30, 4, DarkGrey);
        M5.Display.fillRoundRect(0, 165, 16, 30, 4, DarkGrey);

        g_page = PageSet1;
        dispPageSet1();
        return;
      } else if (g_touch.y >= 105 - 15 && g_touch.y < 135 + 15) {
        g_select = 1;
        M5.Display.fillRoundRect(0, 45, 16, 30, 4, DarkGrey);
        M5.Display.fillRoundRect(0, 105, 16, 30, 4, Orange);
        M5.Display.fillRoundRect(0, 165, 16, 30, 4, DarkGrey);

        g_page = PageSet1;
        dispPageSet1();
        return;
      } else if (g_touch.y >= 165 - 15 && g_touch.y < 180) {
        g_select = 2;
        M5.Display.fillRoundRect(0, 45, 16, 30, 4, DarkGrey);
        M5.Display.fillRoundRect(0, 105, 16, 30, 4, DarkGrey);
        M5.Display.fillRoundRect(0, 165, 16, 30, 4, Orange);

        g_page = PageSet1;
        dispPageSet1();
        return;
      }
    }
  }

  // Encoder
  M5.Display.setTextColor(White, Black);
  M5.Display.setFreeFont(&FreeSans9pt7b);
  if (g_select == 0) {
    for (int i = 1; i < 4; i++) {
      if (checkEncoder(7 - i, -12, 12)) {
        g_setChord[i] = g_curEncoder[7 - i];
        M5.Display.fillRect(112 + i * 48, 45, 32, 30, Black);
        M5.Display.drawRect(112 + i * 48, 45, 32, 30, DarkCyan);
        M5.Display.drawString(String(g_setChord[i]), 128 + i * 48, 60);
      }
      if (checkEncoder(3 - i, 0, 2)) {
        g_setDur[i] = g_curEncoder[3 - i];
        M5.Display.fillRect(112 + i * 48, 75, 32, 30, Black);
        M5.Display.drawRect(112 + i * 48, 75, 32, 30, Blue);
        M5.Display.drawString(String(NoteDurName[g_setDur[i] + 4]), 128 + i * 48, 90);
      }
    }
  } else if (g_select == 1) {
    for (int i = 0; i < 4; i++) {
      if (checkEncoder(7 - i, 0, 127)) {
        g_setLevel[i] = g_curEncoder[7 - i];
        M5.Display.fillRect(112 + i * 48, 105, 32, 30, Black);
        M5.Display.drawRect(112 + i * 48, 105, 32, 30, Purple);
        M5.Display.drawString(String(g_setLevel[i]), 128 + i * 48, 120);
      }
      if (checkEncoder(3 - i, 0, 15)) {
        g_setCh[i] = g_curEncoder[3 - i];
        M5.Display.fillRect(112 + i * 48, 135, 32, 30, Black);
        M5.Display.drawRect(112 + i * 48, 135, 32, 30, Red);
        M5.Display.drawString(String(g_setCh[i] + 1), 128 + i * 48, 150);
      }
    }
  } else {
    for (int i = 0; i < 4; i++) {
      if (checkEncoder(7 - i, 0, 127)) {
        g_setPc[i] = g_curEncoder[7 - i];
        M5.Display.fillRect(112 + i * 48, 165, 32, 30, Black);
        M5.Display.drawRect(112 + i * 48, 165, 32, 30, Green);
        M5.Display.drawString(String(g_setPc[i] + 1), 128 + i * 48, 180);
        if (i % 2 == 0) {
          M5.Display.fillRect(80 + i * 48, 195, 96, 22, Black);
          M5.Display.drawString(String(GmInstruments[g_setPc[i]]), 128 + i * 48, 210);
        } else {
          M5.Display.fillRect(80 + i * 48, 217, 96, 22, Black);
          M5.Display.drawString(String(GmInstruments[g_setPc[i]]), 128 + i * 48, 227);
        }
        g_midi.setInstrument(0, g_setCh[i], g_setPc[i]);
      }
    }
  }
  M5.Display.setFreeFont(&FreeSans12pt7b);


#ifdef Debug
  // Capture screen
  if (!g_encoder.getButtonStatus(0)) {
    delay(500);
    saveBMP("/disp_set1.bmp");
    delay(500);
  }
#endif
}

// ************************************************** **************************************************
// dispPageSet2
// ************************************************** **************************************************
void dispPageSet2() {
  Serial.println("dispPageSet2");

  M5.Display.clear();
  M5.Display.setTextDatum(MC_DATUM);  // MC_DATUM : 中央、ML_DATUM : 左中央

  // Title
  M5.Display.setTextColor(GREENYELLOW, BLACK);
  M5.Display.drawString(SrcId, 64, 16);

  // Text
  M5.Display.setTextColor(White, Black);

  M5.Display.drawString("Key", 64, 60);
  M5.Display.drawString("Note", 160, 60);


  // Input area
  M5.Display.drawRect(32, 75, 64, 30, Blue);

  M5.Display.drawString(String(g_tuneKey), 64, 90);
  M5.Display.drawString(String(NoteName[(g_tuneKey + 120) % 12]) + String((g_tuneKey >= 0) ? g_tuneKey / 12 - 1 : (g_tuneKey - 11) / 12 - 1), 64, 120);

  // On/Off Button
  M5.Display.fillRoundRect(128, 75, 64, 30, 4, (g_tuneOn) ? Orange : DarkGrey);
  M5.Display.setTextColor(White, (g_tuneOn) ? Orange : DarkGrey);
  M5.Display.drawString("On", 160, 90);
  M5.Display.fillRoundRect(224, 75, 64, 30, 4, (g_tuneOn) ? DarkGrey : Orange);
  M5.Display.setTextColor(White, (g_tuneOn) ? DarkGrey : Orange);
  M5.Display.drawString("Off", 256, 90);

  // Encoder
  g_encoder.setEncoderValue(7, g_tuneKey * 2);
  g_curEncoder[7] = g_tuneKey;
  delay(10);

  // Menu Button
  M5.Display.fillRoundRect(0, 210, 64, 30, 4, DarkGrey);
  M5.Display.setTextColor(White, DarkGrey);
  M5.Display.drawString("Home", 32, 226);

  M5.Display.fillRoundRect(80, 210, 64, 30, 4, DarkGrey);
  M5.Display.setTextColor(White, DarkGrey);
  M5.Display.drawString("Ret", 112, 226);

  // LED
  g_encoder.setLEDColor(7, LedCyan);
  delay(10);
  g_encoder.setLEDColor(6, LedBlack);
  delay(10);
  g_encoder.setLEDColor(5, LedBlack);
  delay(10);
  g_encoder.setLEDColor(4, LedBlack);
  delay(10);
  g_encoder.setLEDColor(3, LedBlack);
  delay(10);
  g_encoder.setLEDColor(2, LedBlack);
  delay(10);
  g_encoder.setLEDColor(1, LedBlack);
  delay(10);
  g_encoder.setLEDColor(0, LedBlack);
  delay(10);
}

// ************************************************** **************************************************
// cntlPageSet2
// ************************************************** **************************************************
void cntlPageSet2() {
  if (checkTouch()) {
    Serial.println("cntlPageSet2 - Touch");
    if (g_touch.y >= 210) {
      if (g_touch.x < 64) {  // Home Button
        g_page = PagePlay;
        dispPagePlay();
        return;
      } else if (g_touch.x >= 80 && g_touch.x < 144) {  // Return Button
        g_page = PageSet1;
        dispPageSet1();
        return;
      }
    }

    // On/Off Button
    if (g_touch.y >= 75 && g_touch.y < 105) {
      if (g_touch.x >= 128 && g_touch.x < 192) {
        // Note On
        for (int i = 0; i < 4; i++) {
          g_midi.setInstrument(0, g_setCh[i], g_setPc[i]);
          g_midi.setNoteOn(g_setCh[i], g_tuneKey, g_setLevel[i]);
        }
        M5.Display.fillRoundRect(128, 75, 64, 30, 4, Orange);
        M5.Display.setTextColor(White, Orange);
        M5.Display.drawString("On", 160, 90);
        M5.Display.fillRoundRect(224, 75, 64, 30, 4, DarkGrey);
        M5.Display.setTextColor(White, DarkGrey);
        M5.Display.drawString("Off", 256, 90);

        return;
      } else if (g_touch.x >= 224 && g_touch.x < 288) {
        // Note Off
        for (int i = 0; i < 4; i++) {
          g_midi.setNoteOff(g_setCh[i], g_tuneKey, g_setLevel[i]);
        }
        M5.Display.fillRoundRect(128, 75, 64, 30, 4, DarkGrey);
        M5.Display.setTextColor(White, DarkGrey);
        M5.Display.drawString("On", 160, 90);
        M5.Display.fillRoundRect(224, 75, 64, 30, 4, Orange);
        M5.Display.setTextColor(White, Orange);
        M5.Display.drawString("Off", 256, 90);

        return;
      }
    }
  }

  // Encoder
  M5.Display.setTextColor(White, Black);

  if (checkEncoder(7, 0, 96)) {
    int g_tuneKey = g_curEncoder[7];
    M5.Display.drawRect(32, 75, 64, 30, Blue);

    M5.Display.drawString(String(g_tuneKey), 64, 90);
    M5.Display.drawString(String(g_tuneKey), 64, 120);
  }


#ifdef Debug
  // Capture screen
  if (!g_encoder.getButtonStatus(0)) {
    delay(500);
    saveBMP("/disp_set2.bmp");
    delay(500);
  }
#endif
}

// ************************************************** **************************************************
// setup
// ************************************************** **************************************************
void setup() {

  auto cfg = M5.config();
  M5.begin(cfg);

  //
  Serial.begin(115200);
  g_startMillis = millis();

  // 8Encoderの初期化
  Wire.begin(2, 1);                             // PORT A : SDA=2, SCL=1
  if (!g_encoder.begin(&Wire, ENCODER_ADDR)) {  // I2Cアドレス 0x41
    Wire.end();
    delay(10);
    esp_restart();
  }

  // タイマー設定
  esp_timer_create_args_t timer_args = {};
  timer_args.callback = &onTimer;  // 割り込み時のコールバック関数
  timer_args.name = "timerTick";

  ESP_ERROR_CHECK(esp_timer_create(&timer_args, &g_timer));  // タイマーを作成

  // MIDI初期設定
  Serial2.begin(MIDI_BAUD, SERIAL_8N1, 18, 17);
  g_midi.begin(&Serial2, MIDI_BAUD, 18, 17);

  for (int i = 0; i < 16; i++) {
    g_midi.setAllNotesOff(i);  // All Note Off
  }

  // 変数初期化
  for (int i = 0; i < 26; i++) {
    g_seqLen[i] = 1;
  }

  for (int i = 0; i < 26; i++) {
    for (int j = 0; j < 4; j++) {
      g_patRtm[i][j] = 1;
    }
  }

  setScale();

  //　初期画面表示
  M5.Display.setRotation(1);
  M5.Display.setTextDatum(MC_DATUM);  // MC_DATUM : 中央、ML_DATUM : 左中央
  M5.Display.setFreeFont(&FreeSans12pt7b);
  dispPagePlay();

  M5.Display.setTextColor(White, Black);
  M5.Display.setFreeFont(&FreeSans9pt7b);
  M5.Display.drawString("Ver." + SrcVer, 144, 16);
  M5.Display.setFreeFont(&FreeSans12pt7b);
}

// ************************************************** **************************************************
// loop
// ************************************************** **************************************************
void loop() {
  // Clock
  if (g_internalClock) {
    // Internal Clock

    if (g_timerTriggered) {
      g_timerTriggered = false;

      evalTick();

      g_midi.sendClock();
    }
  } else {
    // External Clock
    while (Serial2.available()) {
      byte data = Serial2.read();

      switch (data) {
        case 0xFA:  // Midi Start
          g_tickCount = 0;
          g_playMode = MidiPlay;
          M5.Display.fillRoundRect(256, 0, 64, 30, 4, Green);
          M5.Display.fillRect(278, 5, 7, 20, BLACK);
          M5.Display.fillRect(291, 5, 7, 20, BLACK);

          break;

        case 0xFC:  // Midi Stop
          g_tickCount = 0;
          g_playMode = MidiStop;
          M5.Display.fillRoundRect(256, 0, 64, 30, 4, DARKGREY);
          M5.Display.fillTriangle(278, 5, 298, 15, 278, 25, BLACK);

          M5.Display.fillCircle(5, 15, 5, BLACK);  // インジケーター消灯

          break;

        case 0xF8:  // Midi Clock
          evalTick();

          break;
      }
    }
  }

  // User Interface
  M5.update();

  // Page
  switch (g_page) {
    case PagePlay:
      cntlPagePlay();
      break;

    case PageSong:
      cntlPageSong();
      break;

    case PageSeq:
      cntlPageSeq();
      break;

    case PageSeqRnd:
      cntlPageSeqRnd();
      break;

    case PagePat:
      cntlPagePat();
      break;

    case PagePatRnd:
      cntlPagePatRnd();
      break;

    case PageSet1:
      cntlPageSet1();
      break;

    case PageSet2:
      cntlPageSet2();
      break;

    case PageFile:
      cntlPageFile();
      break;

    case PageLoad:
      cntlPageLoad();
      break;

    case PageSave:
      cntlPageSave();
      break;

    case PageRename:
      cntlPageRename();
      break;

    case PageDelete:
      cntlPageDelete();
      break;

    case PageInput:
      cntlPageInput();
      break;

    default:
      break;
  }
}

// ************************************************** **************************************************
// saveBMP : Debug用
// ************************************************** **************************************************
#ifdef Debug
void saveBMP(const char *filename) {
  if (!SD.begin(SD_CS)) {
    Serial.println("カードのマウントに失敗しました");
    return;
  }

  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("SDカードが挿入されていません");
    return;
  }

  File file = SD.open(filename, FILE_WRITE);

  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }

  int16_t w = M5.Display.width();
  int16_t h = M5.Display.height();

  // BMPヘッダーを書き込む（24bit RGB）
  uint32_t rowSize = ((24 * w + 31) / 32) * 4;
  uint32_t fileSize = 54 + rowSize * h;

  // BITMAPFILEHEADER
  file.write('B');
  file.write('M');
  file.write((uint8_t *)&fileSize, 4);
  file.write((uint8_t)0);
  file.write((uint8_t)0);
  file.write((uint8_t)0);
  file.write((uint8_t)0);
  file.write((uint8_t)54);
  file.write((uint8_t)0);
  file.write((uint8_t)0);
  file.write((uint8_t)0);

  // BITMAPINFOHEADER
  uint32_t infoSize = 40;
  file.write((uint8_t *)&infoSize, 4);
  file.write((uint8_t *)&w, 4);
  int32_t negH = -h;  // 上から下に描画
  file.write((uint8_t *)&negH, 4);
  uint16_t planes = 1;
  uint16_t bits = 24;
  file.write((uint8_t *)&planes, 2);
  file.write((uint8_t *)&bits, 2);
  for (int i = 0; i < 24; i++) file.write((uint8_t)0);

  // ピクセルデータ
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      uint16_t color = M5.Display.readPixel(x, y);
      uint8_t r = ((color >> 11) & 0x1F) << 3;
      uint8_t g = ((color >> 5) & 0x3F) << 2;
      uint8_t b = (color & 0x1F) << 3;
      file.write(b);
      file.write(g);
      file.write(r);
    }
    // パディング
    while ((w * 3) % 4 != 0) {
      file.write((uint8_t)0);
      w++;
    }
  }

  file.close();
  Serial.println("Screenshot saved");
}
#endif
// ************************************************** **************************************************
