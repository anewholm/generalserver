/*
 * do we really need the OO redirect to work at the $ level?
 * it would mean that JQueryClass methods would need a
 *   OOredirect() call in them somewhere
 *   rather than simply just doing stuff to the DOM
 * can jQuery look at the OO more? NO
 */

//-------------------------
function JInherit(j){
  this.elements = new jQuery(j);
}

//inherit only jQuery expandos
//update OO objects after DOM changes
JInherit.inheritJ = function inheritJ() {
  for (var s in jQuery.fn) JInherit.prototype[s] = new Function("return JInherit.redirect(this,'" + s + "',arguments);");
}
JInherit.inheritJ();
JInherit.redirect = function redirect(t, s, a) {
  jQuery.prototype[s].apply(t.elements, a);
  return this;
}

//-------------------------
function JQueryClass(j){
  //override holder for JInherit
  JInherit.apply(this, arguments);
}
JQueryClass.prototype = new JInherit();
JQueryClass.prototype.show = function show(){
  JInherit.prototype.show.apply(this, arguments);
  return this;
}

//-------------------------
function JOO(j){
  //masquerade as JOO.fMasqueradeClass
  //inherit all OO expandos (includes JInherit => jQuery expandos)
  //allow OO overrides of jQuery functions
  if (!(this instanceof JOO)) return new JOO(j);
  
  JOO.fMasqueradeClass.apply(this, arguments); //=> gives JInherit => this.elements
}
for (var s in jQuery.fn) JOO.prototype[s] = new Function("return JOO.redirect(this,'" + s + "', arguments);");

JOO.redirect = function redirect(t, s, a){
  var oObject, oObjects = new Object();
  var aNonObjects, jNonObjects, aThisArguments, jThis;
  var fFunc = JOO.fMasqueradeClass.prototype[s];
  
  t.elements.each(function(){
    jThis = new jQuery(this);
    if ((oObject = jThis.data().object) && oObject[s] instanceof Function) {
      //collect object elements by object
      if (oObject.aRedirectCollection) oObject.aRedirectCollection.push(this);
      else oObject.aRedirectCollection = [this];
      oObjects[oObject.id] = oObject;
    } else if (fFunc) {
      //collect non-object DOM elements
      if (!aNonObjects) aNonObjects = new Array(this);
      else aNonObjects.push(this);
    }
  });
  
  //JOO.fMasqueradeClass on non-object DOM elements
  if (aNonObjects) {
    jNonObjects = new JOO.fMasqueradeClass(aNonObjects);
    fFunc.apply(jNonObjects, a);
  }
    
  //each object (if any) with its multiple elements
  aThisArguments.unshift(null);
  for (sID in oObjects) {
    oObject = oObjects[sID];
    jThis = new jQuery(oObject.aRedirectCollection);
    aThisArguments[0] = jThis;
    oObject[s].apply(oObject, aThisArguments);
    delete oObject.aRedirectCollection;
  }
  
  return t;
}
JOO.fMasqueradeClass = window.JQueryClass;
$ = JOO;

function Might(sID) {
  this.id = sID;
}
Might.prototype.hide = function hide(){
  console.info("no!");
}

//------------------------- THUS
/*
function MyNewJQueryClass(){
  //override holder for JInherit
  this.prototype.load = blah...
}
MyNewJQueryClass.prototype = new JOO();  //inherits MyNewJQueryClass => JQueryClass => JInherit
$.fMasqueradeClass = MyNewJQueryClass; //total re-use of new overridden JQueryClass
$ = MyNewJQueryClass;              //ignore OO completely
$$ = JOO.new(MyNewJQueryClass);
*/