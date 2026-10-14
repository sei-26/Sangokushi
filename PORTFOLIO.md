# 群雄戦線

## 紹介

都市と武将を管理し、領土と補給線を考えて命令を出す三国志戦略ゲーム。

環境: C++ / OpenSiv3D 0.6.16

## デモの流れ

全国勢力図を表示し、都市選択、出陣、進路、補給の順で説明する。

ウィンドウ表示: $env:SANGOKUSHI_WINDOWED="1"。作業ディレクトリをAppにしてSangokushi.exeを起動。

## 実装を説明する入口

Main.cpp、NewGame/PreviewModes.hpp、CampaignSupply.cpp、CampaignAIPlanning.cpp、CampaignTurn.cpp

## 今回の改善

プレビュー選択の一覧化と1280×720のウィンドウ起動を追加。保存形式・戦略ルール・既存の画面番号は維持。

## 確認した範囲

Releaseビルド、全国図の起動、tests/run.ps1の全試験を確認。

## 提出前の確認

制作期間、本人の担当の細目、素材の利用表記。READMEの旧英雄譚説明は現行メニューと異なる。

主要作品の制作形態は本人申告により主に個人制作。制作期間は本人が追記する。
