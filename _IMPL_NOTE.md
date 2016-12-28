# 実装ノート

## OSPFパケットの実装
OSPFヘッダは`Header`クラスの派生クラス`OSPFHeader`を抽象基底クラスとする。

## LSAの実装
LSAは、`Header`クラスの派生クラス`LSAHeader`を抽象基底クラスとする。

## 実装の依存関係

- OSPFパケットの実装
    - Link State Updateの実装のためにLSAを実装しなければならない
    - 必要なLSAの洗い出しが必要になる
    - Originating LSAの項目を読む