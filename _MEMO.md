# インターフェースに関するデータ構造
https://tools.ietf.org/html/rfc2328#page-63
https://tools.ietf.org/html/rfc5340#section-4.1.2

## Type
point-to-point | broadcast | NBMA | Point-to-MultiPoint | virtual link
## State
- Down
    - InterfaceUpイベントでPoint-to-pointになる
- Point-to-point
他は一旦無視
- Loopback
- Waiting
- DR Other
- Backup
- DR

## Interface ID
v3
## Instance ID
v3
## List of LSAs with link-local scope
v3
## IP interface address
v3
## IP interface mask
v3では不在では？
## Area ID
面しているネットワークのArea ID
## HelloInterval
ハローメッセージ送信間隔
## RouterDeadInterval
ネイバーの死活判定基準時間。最後にハローメッセージが来てからこの時間経過すると死んだと見なす。
## InfTransDelay
このインターフェースからLSUパケットを送信するのにかかる秒数の推定値。LSUパケットに含まれるLSAそれぞれのLSageの値は、この値のぶんだけ送信前に増加される。この値は送信と伝播遅延を考慮し、また0以上でなければならない。
## Router Priority
uint8。DR選出時に最もこの値の高いものが選出される。0になっているとDRにはならない。ハローパケットに含まれて伝達される。
## Hello Timer
ハローパケットを送信するためのタイマー。HelloIntervalごと定期的に発火する。非ブロードキャストなネットワーク上では、ハローパケットは個別に送信を行う。
## Wait Timer
WaitingのStateから抜けるためのタイマー。RouterDeadIntervalの値を使う。
## List of neighboring routers
ネイバールータの一覧。ハローパケットによって生成される。近隣ルータはこの中の幾つかが選ばれることになる。
## Designated Router
## Backup Designated Router
## Interface output cost(s)
router-LSAのlink costとして伝達される。0以上の値をとる。
## RxmtInterval
近隣ルータへのLSAの再送信間隔秒数。Database DescriptionとLink State Requestの再送信にも使われる。
## AuType
## Authentication key
## List of link prefixes
参加しているリンクのために設定されるIPv6のプレフィクスリスト。link-LSAによって伝達され、リンクのDRによってintra-area-prefix-LSAによって伝達されうる。



# ネイバーに関するデータ構造
## State
ネイバーの状態です。

## Inactivity Timer
ハローパケットが来るたびにリセットされるタイマーです。初期値はRouterDeadIntervalです。

## Master/Slave
ExStart時に決定されます。LSDBの交換プロセスで使います。

## DD Sequence Number
ネイバーに送信中のDatabaseDescriptionPacketのシーケンス番号を保持します。

## Last received Database Description packet
最後に受け取ったDDパケット全体を保持します。

## Neighbor ID
ネイバーのルータIDです。最初にハローパケットを受け取った時に知ることができます。

## Neighbor Priority
同様

## Neighbor's Interface ID


## Neighbor IP address
ネイバーのIPアドレスはOSPFパケットのソースアドレスとして含まれているはずです。これは仮想リンクでない限り、IPv6のリンクローカルアドレスになります。

## Neighbor's Designated Router
## Neighbor's Backup Designated Router
対応するIPアドレスでしたが、v3ではルータIDを保持します。

## Link State Retransmission List
各ルータは、アジャセンシーに対して一つずつこのリストを持つ。
これはフラッディングしたが確認応答がきていないLSAのリストである。
確認応答がくるか、アジャセンシーが死ぬとこのリストから消える。

## Link State Request List
各ルータは、ネイバーに対して一つずつこのリストを生成する。
ネイバーとのLSDBの同期を行うために使う、期限切れなどにより改めて受け取るべきLSAのリストである。
このリストはDatabaseDescriptionパケットの受信によって生成され、
ExchangeDoneの後にこのリストにアイテムがあれば、それを受け取るためのLinkStateRequestパケットが送信される。
このリストは適切なLinkStateUpdateパケットの受信によって消化されていく。

## Database Summary List
LSDBを構成する完全なLSAのリストとして、ネイバーがDatabase Exchange状態（つまりExStartのタイミング）になったときに作成される。
このリストがDatabaseDescriptionパケットによって送信される。確認応答が得られると、送信した分は削除される。



OSPF packetのやりとりに関する図
https://tools.ietf.org/html/rfc2328#page-106

Database Description packetのM/SはルータIDが大きい方がMaster

```
+---+                                         +---+
|RT1|                                         |RT2|
+---+                                         +---+

Down                                          Down
              Hello(DR=0,seen=0)
         ------------------------------>
           Hello (DR=RT2,seen=RT1,...)      Init
         <------------------------------
ExStart      D-D (Seq=x,I,M,Master)
         ------------------------------>
             D-D (Seq=y,I,M,Master)         ExStart
         <------------------------------
Exchange       D-D (Seq=y,M,Slave)
         ------------------------------>
             D-D (Seq=y+1,M,Master)         Exchange
         <------------------------------
             D-D (Seq=y+1,M,Slave)
         ------------------------------>
                      ...
                      ...
                      ...
              D-D (Seq=y+n, Master)
         <------------------------------
              D-D (Seq=y+n, Slave)
Loading  ------------------------------>
                   LS Request                   Full
         ------------------------------>
                   LS Update
         <------------------------------
                   LS Request
         ------------------------------>
                   LS Update
         <------------------------------
Full

```


rfc2328 Section. 13 The Flooding Procedure

Link State Updateパケットは、LSAをフラッディングするメカニズムを提供します。LinkStateUpdateパケットは、いくつかの別個のLSAを含むことができ、それぞれのLSAをその発信点からさらに遠くに伝えます。フラッディング手順を信頼できるものにするには、各LSAを個別に確認する必要があります。肯定応答は、リンク状態肯定応答パケットで送信されます。多くの個別の確認応答をまとめて1つのパケットにまとめることもできます。
フラッディング手順は、LinkStateUpdateパケットが受信されたときに開始されます。受信されたパケットは、フラッディング手順に渡される前に、多くの整合性チェックが行われています（8.2項を参照）。特に、LinkStateUpdateパケットは、特定のネイバーおよび特定のareaに関連付けられています。ネイバーがExchange未満状態にある場合、パケットはそれ以上の処理を行わずにドロップする必要があります。
AS外部LSA以外のすべてのタイプのLSAは、特定の領域に関連付けられています。ただし、LSAには領域フィールドは含まれていません。 LSAのエリアはLinkStateUpdateパケットヘッダーから導かれなければなりません。
LSUパケットに含まれるLSAのそれぞれに対して次の手順が取られます：
1. LSAのLSチェックサムを確認します。不正であれば破棄して次のLSAを見ます。
2. LSAのLSタイプを確認します。unknownであれば破棄して次のLSAを見ます。
3. あるいは、LSタイプが5(AS-external-LSA)であれば……（今回は無視）
4. あるいは、LSAのageがMaxAgeと等しく、かつルータのLSDBにそのLSAが存在せず、さらに近隣ルータにExchangeかLoading状態なものが存在しない場合、次の手順をとります。
    1. LSAckを近隣ルータに送信します。
    2. LSAを破棄して次のLSAを見ます。
5. LSDBにLSAのインスタンスがあるか探します。存在しないか、今受け取ったLSAの方がより新しい場合（新しさの確認方法はSection13.1で）は、次の手順をとります。
    1. すでにコピーが存在して、フラッディングにより受け取ったものであって、それが追加されてからMinLSArrival経っていない場合、破棄して次のLSAを見ます。
    2. そうでなければこのLSAをすぐにインターフェースからフラッディングします。
    3. 全てのリンク状態再送信リストから現在のDBのコピーを削除します。
    4. 新しいLSAをLSDBにインストールします。ルーティングテーブルの再計算がスケジューリングされるかもしれません。新たなLSAの追加時刻はこの段階での時刻を使います。フラッディング機構はここでインストールされたLSAをMinLSArrival秒が経過するまで上書きできません。LSAのインストールについてはSection 13.2を参照してください。
    5. 場合によってはLSAを受け取った証としてLSAckを、受信したインターフェースから送り返すかもしれません。Section13.5を参照してください。
        1. 1で既存のものより新しいLSAを受け取っており、2でフラッディングが行われた場合は、LSAckを受信したインターフェースから送り返す。
        2. 2でフラッディングが行われた場合はLSAckは送らない。
    6. もしこの新たなLSAが自分自身が発行したものであった場合、なんかよくわかんないから実装しない
6. 送信中近隣リンク状態リクエストリストの中にLSAを見つけた場合、データベース交換のプロセスにエラーが生じているということです。この場合は、近隣イベントのBadLSReqを送信してLSUの処理を中断させることにより、データベース交換プロセスを再始動します。
7. 全く同じインスタンスをLSDBの中に見つけた場合、次の2つの手順で処理されます。
    1. LSAがリンク状態再送信リストの中にある場合、このルータ自身がこのLSAに対する確認応答を待っているところです。ルータは受け取ったLSAを確認応答として扱い、リンク状態再送信リストから削除します。これは「暗黙の確認応答」と呼ばれるものです。確認応答プロセスSection13.5を参照してください。
    2. 場合によってはLSAを受け取った証としてLSAckを、受信したインターフェースから送り返すかもしれません。Section13.5を参照してください。
        1. 暗黙の確認応答として扱った場合、BDRはなんかいろいろやって送り返し、そうでなければ送らない。
        2. 暗黙の確認応答として扱わない場合、直接確認応答を送り返す。
8. LSDB内のデータの方が新しい場合、DB内のもののLS ageがMaxAgeと等しくかつLSシーケンス番号がMaxSequenceNumberに等しい場合は、単に応答なく破棄します。そうでない場合は、直近のMinLSArrival秒からまだ送信されていないぶんを、LSUとして当該近隣ルータに送信します。このときLSUは近隣ルータに直接送信する必要があり、これをリンク状態再送信リストには入れず、確認応答は送信しないでください。


# LSAの生成について

OSPF for IPv6 4.4.3.  Originating LSAs
https://tools.ietf.org/html/rfc5340#section-4.4.3

OSPF v2 12.4 Originating LSAs
https://tools.ietf.org/html/rfc2328#page-123

10 + 7個のイベントによってLSAインスタンスが生成されうる

1. ルータ自身が生成したLSAのLSageがLSRefreshTimeに到達したとき。
    - このときLSage以外の値は同じでよい。
2. Interfaceの状態が変わったとき。
    - 新たなrouter-LSAを生成する。
3. 所属するネットワークのDesignated Routerが変わったとき。
    - 新たなrouter-LSAを生成する。
    - 自身が新たなDRとなる場合は、network-LSAも生成する。
    - 自身がDRでなくなった場合は、network-LSAは生成しない。
4. ネイバーのひとつがFULLになる、または自分がなったとき。
    - 新たなrouter-LSAを生成する。
    - 自身がDRである場合は、network-LSAも生成する。
5. (考慮外)エリア境界ルータにおいて、エリア内経路が追加/削除/変更されたとき。
    - 操作のあった経路に関するsummary-LSAをエリア内に送信する。
6. (考慮外)エリア境界ルータにおいて、エリア外経路が追加/削除/変更されたとき。
    - 操作のあった経路に関するsummary-LSAをエリア内に送信する。
7. (考慮外)エリア境界ルータにおいて、このルータがエリアに新たに追加されたとき。
    - 適切にエリア内経路とエリア外経路を含んだsummary-LSAを送信する。
8. (考慮外)エリア境界ルータにおいて、ルータの持つ仮想リンクの設定が変更されたとき。
    - 新たなrouter-LSAを仮想リンクに流す。
9. (考慮外)AS境界ルータにおいて、……（省略）
10. (考慮外)AS境界ルータにおいて、……（省略）

以下、v3追加分

1. ルータのインターフェースの状態かIDが変わったとき。
    - link-LSAとrouter-LSAs、and/or intra-area-prefix-LSAsを(再)生成するかflushする。
    - (考慮外)自身がDRなら、関係のあるnetwork-LSAを再生成するかflushする。
2. リンクのDRが変更されたとき。
    - 当該リンクのnetwork-LSAを再生成するかflushする。
    - ひとつか複数のrouter-LSAs and/or intra-area-prefix-LSAsを(再)生成するかflushする。
3. ネイバーがFullの状態になるか、Fullから変化したとき。
    - router-LSAを生成して送信する。
4. ネイバーのインターフェイスIDが変化したとき。
    - router-LSAを生成して送信する。
5. リンクに新たなプレフィクスが追加、または削除、あるいはその両方が行われたとき。
    - リンクに対してlink-LSAを再生成するか、リンクに存在する唯一のルータとなった場合はintra-area-prefix-LSAを再生成する。
6. (考慮外)新たなlink-LSAを受け取って、リンクコレクションのプレフィクスが変化したとき。
    - ルータがリンクのDRならば、intra-area-prefix-LSAを生成する。
7. (考慮外)新たなlink-LSAを受け取って、リンク上の近接ルータが送信するLSAオプションの論理和が変化したとき。
    - ルータがリンクのDRならば、network-LSAを生成する。

intra-area-prefix-LSAはDRが生成する。つまりP2P接続の場合は生成されない。
network-LSAもDRが生成するので同様に生成されない。
つまり喫緊に実装の必要があるのはlink-LSAとrouter-LSAのみ
