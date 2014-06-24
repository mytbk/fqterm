// a single variable batching example

fqterm.importFile("utils.js");

var begin = Number(fqterm.askDialog("Batch", "Variable i: begin", 1));
var end = Number(fqterm.askDialog("Batch", "Variable i: end", 3));
var step = Number(fqterm.askDialog("Batch", "Variable i: step", 1));
var keyStr = fqterm.askDialog("Batch", "Key Sequence:", "%i");
var interval = 100;

if (step>0){
    cmp = function(a,b){return a<=b;};
}else{
    cmp = function(a,b){return a>=b;};
}

for (var i=begin; cmp(i,end); i=i+step){
    var str = keyStr.
        replace(/%%/g, "%\x01").
        replace(/%i/g, i).
        replace(/\x01/g, "");
    fqterm.sendParsedString(str);
    sleep(interval);
}

    
