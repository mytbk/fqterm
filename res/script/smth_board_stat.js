fqterm.importFile("utils.js");

function ParseDate(s) {
    var t = s.split('-');
    return { year:t[0], month:t[1], day:t[2] };
}

function ParseLine(s) {
    if (s[0]=='>') {
        s = s.substr(1);
    }
    var t = s.trim().split(' ');
    var num = t[0];
    var attr = null;
    var name = null;
    if (t[1]!="") {
        attr = t[1];
        name = t[2];
    } else {
        for (var i=2;i<t.length;i++) {
            if (t[i]!="") {
                name = t[i];
                break;
            }
        }
    }
    return { id:num, attr:attr, uname:name };
}

function arrSearch(a,f) {
    for (var i=0; i<a.length; i++) {
        if (f(a[i])==true) {
            return i;
        }
    }
    return -1;
}

function ESCseq(s) {
    return "\x1b\x1b["+s+"m";
}

// first, enter super article search mode
fqterm.sendParsedString("^g7\n");

var start_str = fqterm.askDialog("开始日期","从哪天开始统计(包含该日)？","2015-04-01");
var end_str = fqterm.askDialog("结束日期","统计至哪天(不包含该日)？","2015-05-01");

var startTime = ParseDate(start_str);
var endTime = ParseDate(end_str);
var queryStr = "ftime>=date(" + startTime.year + "," + startTime.month + "," + startTime.day +
    ")&&ftime<=date(" + endTime.year + "," + endTime.month + "," + endTime.day + ")\n";
fqterm.sendString(queryStr);
sleep(100);

// var firstLineNo = 3;

fqterm.sendString("\x1b[4~"); // END
sleep(500);
var lastArt = ParseLine(fqterm.getText(fqterm.caretY()));
//fqterm.msgBox(lastArt.id);

fqterm.sendString("\x1b[1~"); // HOME
sleep(500);

fqterm.sendString("\x1b[6~"); // PGDN
sleep(500);
var secondPage = ParseLine(fqterm.getText(fqterm.caretY()));
var art_per_page = 99;
if (secondPage.id-lastArt.id<0) {
    art_per_page = secondPage.id - 1;
}

fqterm.sendString("\x1b[1~"); // HOME
sleep(500);
var firstLine = fqterm.caretY();

var artid = 1;
var stat_data = new Array();
// var log = "";
while (artid<=lastArt.id) {
    for (var l=firstLine; l<firstLine+art_per_page && artid<=lastArt.id; l++) {
        var cur = ParseLine(fqterm.getText(l));
        //        log += "id=" + cur.id + " author=" + cur.uname + " attr=" + cur.attr + "\n";
        var p = arrSearch(stat_data, function(o){return o.uname==cur.uname});
        if (p!=-1) {
            stat_data[p].artNum++;
            if (cur.attr!=null) {
                if (cur.attr=='m'||cur.attr=='M') {
                    stat_data[p].mNum++;
                } else if (cur.attr=='g'||cur.attr=='G') {
                    stat_data[p].gNum++;
                } else if (cur.attr=='b'||cur.attr=='B') {
                    stat_data[p].mNum++;
                    stat_data[p].gNum++;
                }
            }
        } else {
            stat_data.push({uname:cur.uname,artNum:1,mNum:0,gNum:0});
        }
                    
        artid++;
    }
    fqterm.sendString("\x1b[6~"); // PGDN
    sleep(500);
}

stat_data.sort(function(a,b){return b.artNum-a.artNum;});

// fqterm.artDialog(log);
function showStrWidth(s,len,align) {
    s = String(s);
    var sps = "";
    while (len-s.length>0) {
        sps += " ";
        len--;
    }
    if (align==0) { // left align
        return s+sps;
    } else {
        return sps+s;
    }
}
        
var log2="|------+--------------+--------+--------+--------|\n"
    +    "|  排名|        用户名|    总数|     m文|     g文|\n"
    +    "|------+--------------+--------+--------+--------|\n"
for (var i=0; i<stat_data.length; i++) {
    log2 += "|" + ESCseq("1;44;32") + showStrWidth(i+1,6,1) + ESCseq("") +
        "|" + ESCseq("1;41;36") + showStrWidth(stat_data[i].uname,14,0) + ESCseq("") +
        "|" + ESCseq("1;40;32") + showStrWidth(stat_data[i].artNum,8,1) + ESCseq("") +
        "|" + ESCseq("1;40;32") + showStrWidth(stat_data[i].mNum,8,1) + ESCseq("") +
        "|" + ESCseq("1;40;32") + showStrWidth(stat_data[i].gNum,8,1) + ESCseq("") + "|\n" +
        "|------+--------------+--------+--------+--------|\n";
}


fqterm.artDialog(log2);
