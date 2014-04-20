fqterm.importFile("functions.js");

isexpt=function(num){
    if (num==1){
	return "1";
    }
    for (var i=2;i*i<=num;++i){
	var p=i;
	for (j=1;p<=num;++j){
	    if (p==num){
		var strs = new Array();
		strs[0] = i+"^"+j;
		strs[1] = "(expt "+i+" "+j+")";
                strs[2] = i+"**"+j;
		return strs[rand(3)];
	    }else{
		p *= i;
	    }
	}
    }
    return -1;
}

ismod100=function(num){
    var strs = new Array();
    strs[0] = num;
    if (num%1000==0){
        strs[1] = "000";
	return strs[rand(2)];
    }else if (num%100==0){
        strs[1] = "00";
        return strs[rand(2)];
    }else{
	return -1;
    }
}

isseq = function(num){
    if (num<100){
        return -1;
    }
    var digits = new Array();
    var tmp = num;
    var n = 0;
    for (; tmp>0; n++){
	digits[n] = tmp%10;
	tmp = (tmp-digits[n])/10;
    }
    for (var i=1; i<n-1; ++i){
        if (digits[i]*2==digits[i-1]+digits[i+1]){
            continue;
        }else{
            return -1;
        }
    }
    return num;
}

isz=function(num){
    var str;
    str = ismod100(num);
    if (str!=-1){
        return str;
    }
    str = isexpt(num);
    if (str!=-1){
	return str;
    }
    str = isseq(num);
    if (str!=-1){
	return str;
    }
    return -1;
}


