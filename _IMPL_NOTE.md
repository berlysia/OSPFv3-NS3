# 実装ノート

## OSPFパケットの実装
OSPFヘッダは`Header`クラスの派生クラス`OSPFHeader`を基底クラスとする。

## LSAの実装
LSAは、ヘッダ部を表現する`OSPFLSAHeader`クラスと、`OSPFLSABody`クラスを基底クラスに持つ各派生クラスにより構成される。
これらを束ねる`OSPFLSA`クラスがスマートポインタで参照を持つ。

## 実装の依存関係

- OSPFパケットの実装
    - Link State Updateの実装のためにLSAを実装しなければならない
    - 必要なLSAの洗い出しが必要になる
    - Originating LSAの項目を読む

# 細かい実装の前に見る
https://tools.ietf.org/html/rfc5340#section-4.5
