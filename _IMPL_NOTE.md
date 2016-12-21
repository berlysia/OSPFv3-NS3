# 実装ノート

## OSPFパケットの実装
OSPFヘッダは`Header`クラスの派生クラス`OSPFHeader`とする。
type値による個別の値はByteTagによって持つ。

## LSAの実装
LSAは、LSAHeaderと個別のLSA実装を分離する。
これらはOSPFパケットを表現するTagに属するため、基底クラスは持たない。

## 実装の依存関係

- OSPFパケットの実装
    - Link State Updateの実装のためにLSAを実装しなければならない
    - 必要なLSAの洗い出しが必要になる
    - Originating LSAの項目を読む