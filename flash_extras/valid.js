window.onload = initForm;
var wform;
var productIndex = 1;
var qtyIndex = 1;
var shipIndex = 1;

function initForm() {
   wform = document.forms[0];
   wform.product.onchange = calcCost;
   wform.qty.onchange = calcCost;
   wform.shipping.onchange = calcCost;
   wform.onsubmit = validateForm;
}

function calcCost() {
   productIndex = wform.product.selectedIndex;
   productCost = parseFloat(wform.product.options[productIndex].value);

   qtyIndex = wform.qty.selectedIndex;
   qtyVal = parseFloat(wform.qty.options[qtyIndex].value);

   shipIndex = wform.shipping.selectedIndex;
   shipVal = parseFloat(wform.shipping.options[shipIndex].value);

   if (productIndex != 0 && qtyIndex != 0 && shipIndex != 0) {
      wform.total.value = "$"+(productCost*qtyVal+shipVal).toFixed(2);
   }
}


function validLengths() {
   var isValid = true;
   
   if (testLength(wform.total)== false) isValid = false;
   if (testLength(wform.fname)== false) isValid = false;
   if (testLength(wform.lname)== false) isValid=false;
   if (testLength(wform.address)== false) isValid=false;
   if (testLength(wform.phone)== false) isValid=false;
   if (testLength(wform.cholder)== false) isValid=false;
   if (testLength(wform.cnum)== false) isValid=false;
  
   return isValid;
}

function testLength(field) {
   var isValid = true;
   if (field.value.length == 0) {
      document.getElementById(field.id+"Lab").style.color="red";
      isValid = false;
   } else {
      document.getElementById(field.id+"Lab").style.color="black";
   }
   return isValid;
}

function validPatterns() {
   var isValid = true;

   phonereg = /^\(?\d{3}[\)-]?\s?\d{3}[\s-]?\d{4}$/;
   if (testPattern(wform.phone, phonereg) == false) isValid = false;

   creditType = wform.ccard.selectedIndex;
   switch (creditType) {
      case 0: cregx = /^3[47]\d{13}$/; break;
      case 1: cregx = /^30[0-5]\d{11}$|^3[68]\d{12}$/; break;
      case 2: cregx = /^6011\d{12}$/; break;
      case 3: cregx = /^5[1-5]\d{14}$/; break;
      case 4: cregx = /^4(\d{12}|\d{15})$/; break;
   }  

   //if (testPattern(wform.cnum, cregx) == false) isValid = false;

   return isValid;
}


function testPattern(field, reg) {
   var isValid = true;

   wsregx = /\s/g;
   var fv =field.value.replace(wsregx,"");
   if (reg.test(fv) == false) {
      isValid = false;
      document.getElementById(field.id+"Lab").style.color="red";
   } else {
      document.getElementById(field.id+"Lab").style.color="black";
   }
   return isValid;
}

function validCNum() {
   var isValid = true;

   wsregx = /\s/g;
   var fv = wform.cnum.value.replace(wsregx,"");

   if (luhn(fv)==false) {
      isValid = false;
       document.getElementById("cnumLab").style.color="red";
   } else {
       document.getElementById("cnumLab").style.color="black";
   }

   return isValid;
}


function validateForm() {

   vForm = validLengths() && validPatterns() && validCNum();

   if (!vForm) {
      alert("Check the highlighted items in the form for missing/invalid data");
      return false;
   } else {
      return true;
   }
}