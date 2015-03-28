/*
   New Perspectives on JavaScript, 2nd Edition
   Tutorial 5
   Case Problem 4

   Filename: functions.js
   Author: Roger Blaine
   Date:   3/1/2012

   Functions List:
   todayTxt()
      Displays the current date in the format mm/dd/yyyy.


   addCommas(value)
      Adds comma grouping to a numeric value (###,###,###.###). Returns a
      text string

   dollars(value)
      Converts a numeric value to a current format, including commas and the
      leading "$" symbol. Returns a text string.

   luhn(num)
      Returns a Boolean value indicating whether the number in the num text
      string fulfills the Luhn Formula.

*/


function todayTxt() {
   var Today=new Date();
   return Today.getMonth()+1+"-"+Today.getDate()+"-"+Today.getFullYear();
}


function addCommas(value) {
   valuetxt=value+"";
   decimals="";
   re = /(-?\d+)(\d{3})/;

   decimalpos = valuetxt.search(/\./);
   if (decimalpos != -1) {
      decimals = "."+valuetxt.substring(decimalpos+1,valuetxt.length);
      valuetxt = valuetxt.substring(0,decimalpos);
   }
   while(re.test(valuetxt)) {valuetxt = valuetxt.replace(re,"$1,$2")}

   return valuetxt+=decimals;
}


function dollars(value) {
   n = Math.round(value*100)/100;

   if (n == Math.round(n)) {valuetxt=n+".00"}
   else if (n*10 == Math.round(n*10)) {valuetxt=n+"0"}
   else valuetxt=n+"";

   re = /(-?\d+)(\d{3})/;
   decimalpos = valuetxt.search(/\./);
   if (decimalpos != -1) {
      decimals = "."+valuetxt.substring(decimalpos+1,valuetxt.length);
      valuetxt = valuetxt.substring(0,decimalpos);
   }
   while(re.test(valuetxt)) {valuetxt = valuetxt.replace(re,"$1,$2")}

   return "$"+valuetxt+decimals;
}

function luhn(num) {
   var luhnTotal=0;
   for (i=num.length-1; i>=0; i--) {
      luhnTotal += parseInt(num.charAt(i));
      i--;
      num2 = new String(num.charAt(i)*2);
      for (j=0; j < num2.length; j++) {
         luhnTotal += parseInt(num2.charAt(j));
      }
    }
   return (luhnTotal % 10 == 0);
}