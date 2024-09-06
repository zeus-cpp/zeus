﻿#include "crc64_ecma182_digest_impl.h"
#include <cstdlib>

namespace
{
//crc64-ecma-182:生产多项式:poly = 0xc96c5795d7870f42ULL

uint64_t CRC_TABLE[256] = {
    0ULL,
    12911341560706588527ULL,
    17619267392293085275ULL,
    5164075066763771700ULL,
    8921845837811637811ULL,
    14483170935171449180ULL,
    10328150133527543400ULL,
    4357999468653093127ULL,
    17843691675623275622ULL,
    4940391307328217865ULL,
    226782375002905661ULL,
    12685511915359257426ULL,
    10119945210068853333ULL,
    4566377562367245626ULL,
    8715998937306186254ULL,
    14689403211693301089ULL,
    9051005139383707209ULL,
    14895072503764629798ULL,
    9880782614656435730ULL,
    4193374422961527165ULL,
    453564750005811322ULL,
    13070904082541799189ULL,
    17496296445768931361ULL,
    4747102235666401102ULL,
    9960315520700766767ULL,
    4113029525020509504ULL,
    9132755124734491252ULL,
    14812441257301386523ULL,
    17431997874612372508ULL,
    4811156168024382323ULL,
    391483189436228679ULL,
    13132671735097031464ULL,
    18102010278767414418ULL,
    5195199925788447741ULL,
    1131375642422963401ULL,
    13591081480414639014ULL,
    9288535643022529185ULL,
    3731739485546663374ULL,
    8386748845923054330ULL,
    14361410892855143829ULL,
    907129500011622644ULL,
    13814943346342178715ULL,
    17875617253995106479ULL,
    5421418680781082560ULL,
    8594564625313771207ULL,
    14152643483341451688ULL,
    9494204471332802204ULL,
    3525329033817543155ULL,
    9704381199536204507ULL,
    3855837706121835956ULL,
    8226059050041019008ULL,
    13908973417437222383ULL,
    18265510249468982504ULL,
    5643692520190618503ULL,
    718348998302913715ULL,
    13463047253836762076ULL,
    8146277531524994749ULL,
    13989069943491807698ULL,
    9622312336048764646ULL,
    3938150108875254153ULL,
    782966378872457358ULL,
    13399312233903888353ULL,
    18327840216347633877ULL,
    5582173445676054458ULL,
    7257036000092981153ULL,
    15535280666427316430ULL,
    10390399851576895482ULL,
    2529986302517213333ULL,
    2262751284845926802ULL,
    12414353723947190013ULL,
    16997392145760156105ULL,
    6398650419759490726ULL,
    10599130201908394951ULL,
    2322133910755632296ULL,
    7463478971093326748ULL,
    15329644185724306675ULL,
    16773497691846108660ULL,
    6622864283287239323ULL,
    2036569382881248687ULL,
    12640783567252986560ULL,
    1814259000023245288ULL,
    12250853444207230599ULL,
    17125426475222188467ULL,
    6811676960462675676ULL,
    7132938157145702363ULL,
    15119434731753103540ULL,
    10842837361562165120ULL,
    2690676064372932847ULL,
    17189129250627542414ULL,
    6747026957542163169ULL,
    1875814858707893717ULL,
    12188560364711551674ULL,
    10762704257491731389ULL,
    2770420489343360210ULL,
    7050658067635086310ULL,
    15201536148867841161ULL,
    11493583972846619443ULL,
    3219832958944941148ULL,
    7711675412243671912ULL,
    15576564987190227975ULL,
    16452118100082038016ULL,
    6305011443818121839ULL,
    1213047649942025563ULL,
    11816267669673208372ULL,
    7503259434831574869ULL,
    15784731923736995898ULL,
    11287385040381237006ULL,
    3425713581329221729ULL,
    1436697996605827430ULL,
    11591809733187859977ULL,
    16677985422973077821ULL,
    6078267261889762898ULL,
    16292555063049989498ULL,
    5851447209550246421ULL,
    1630020308903038241ULL,
    11939238787801010766ULL,
    11081681957373440841ULL,
    3090674103720225830ULL,
    7876300217750508306ULL,
    16023932746787097725ULL,
    1565932757744914716ULL,
    12003503911822413427ULL,
    16230825569204842823ULL,
    5913566482019610152ULL,
    7956607163135676207ULL,
    15944361922680361024ULL,
    11164346891352108916ULL,
    3008957496780927003ULL,
    14514072000185962306ULL,
    8809633696146542637ULL,
    4460922918905818905ULL,
    10287960411460399222ULL,
    12879331835779764593ULL,
    113391187501452830ULL,
    5059972605034426666ULL,
    17660565739912801861ULL,
    4525502569691853604ULL,
    10224187249629523019ULL,
    14576435430675780479ULL,
    8748148222884465680ULL,
    4980157760350383383ULL,
    17740628527280140920ULL,
    12797300839518981452ULL,
    195741594718114339ULL,
    13040162471224305931ULL,
    565687821211481700ULL,
    4644267821511264592ULL,
    17536326748496696895ULL,
    14926957942186653496ULL,
    8937808626997553239ULL,
    4297282312656885603ULL,
    9839608450464401420ULL,
    4852190599768102253ULL,
    17327666750234135042ULL,
    13245728566574478646ULL,
    359174499151456857ULL,
    4073138765762497374ULL,
    10063573324157604913ULL,
    14700457781105076997ULL,
    9163920108173816938ULL,
    3628518000046490576ULL,
    9328460452529085631ULL,
    14330211790445699979ULL,
    8498696072880078052ULL,
    5299565100954197475ULL,
    18061012165519327884ULL,
    13623353920925351352ULL,
    1018284691440624343ULL,
    14265876314291404726ULL,
    8562713237611094233ULL,
    3566469078572851181ULL,
    9390260331795218562ULL,
    13702854325316886917ULL,
    937907429353946858ULL,
    5381352128745865694ULL,
    17978417549248290481ULL,
    5746791986423309721ULL,
    18225777846762470134ULL,
    13494053915084326338ULL,
    606523824971012781ULL,
    3751629717415787434ULL,
    9745292510640121029ULL,
    13876787882151992305ULL,
    8338992711486538910ULL,
    13285957365033343487ULL,
    815010154451519120ULL,
    5540840978686720420ULL,
    18431906428167644875ULL,
    14101316135270172620ULL,
    8115412784602421411ULL,
    3978303581567838103ULL,
    9519354766961195256ULL,
    12527462061959317731ULL,
    2230461459452909452ULL,
    6439665917889882296ULL,
    16893009583564617687ULL,
    15423350824487343824ULL,
    7288217715337890239ULL,
    2490078880175191691ULL,
    10493603952060017124ULL,
    6520081235612152965ULL,
    16813546994155744234ULL,
    12610022887636243678ULL,
    2148641156328442801ULL,
    2426095299884051126ULL,
    10557972909709735385ULL,
    15361512820870335213ULL,
    7350228890552538498ULL,
    15006518869663149738ULL,
    7165105895222849989ULL,
    2649782550477098737ULL,
    10947027550912647582ULL,
    12362696414880903321ULL,
    1783234539286425590ULL,
    6851427162658443458ULL,
    17022309211647725485ULL,
    2873395993211654860ULL,
    10722532847870938531ULL,
    15232418832718623383ULL,
    6938393941075996152ULL,
    6642978682516671743ULL,
    17230443782969840528ULL,
    12156534523779525796ULL,
    1989151790783919051ULL,
    6263731030979658865ULL,
    16556202624882645790ULL,
    11702894419100492842ULL,
    1245039440087595845ULL,
    3260040617806076482ULL,
    11390642587947386157ULL,
    15688795063501830681ULL,
    7680756410435167606ULL,
    11622868312827688983ULL,
    1324891275238549368ULL,
    6181348207440451660ULL,
    16638201170595874595ULL,
    15752600435501016612ULL,
    7616209416359311691ULL,
    3321489341258335871ULL,
    11328242235714328848ULL,
    3131865515489829432ULL,
    10977756817953029463ULL,
    16137146508898304611ULL,
    7844397531750915340ULL,
    5811434156413844491ULL,
    16395372229761246052ULL,
    11827132964039220304ULL,
    1660744670629167935ULL,
    15913214326271352414ULL,
    8068573254449152305ULL,
    2905717078206922245ULL,
    11204220263579804010ULL,
    12035829987123708013ULL,
    1452858539103461122ULL,
    6017914993561854006ULL,
    16189773752444600153ULL};
}

namespace zeus
{
Crc64Ecma182DigestImpl::Crc64Ecma182DigestImpl()
{
    Reset();
}
Crc64Ecma182DigestImpl::~Crc64Ecma182DigestImpl()
{
}
const std::byte* Crc64Ecma182DigestImpl::Digest()
{
    return reinterpret_cast<std::byte*>(&_hash);
}
void Crc64Ecma182DigestImpl::Reset()
{
    _hash = 0;
}
size_t Crc64Ecma182DigestImpl::GetSize()
{
    return sizeof(uint64_t);
}
void Crc64Ecma182DigestImpl::UpdateImpl(const void* input, size_t length)
{
    _hash = ~_hash;

    const unsigned char* data = reinterpret_cast<const unsigned char*>(input);
    int                  idx  = 0;
    while (length)
    {
        _hash = CRC_TABLE[((int32_t) (_hash ^ data[idx])) & 0xff] ^ (_hash >> 8);
        idx++;
        length--;
    }
    _hash = ~_hash;
}
} // namespace zeus