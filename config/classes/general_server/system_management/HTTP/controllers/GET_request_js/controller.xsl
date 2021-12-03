<xsl:stylesheet response:server-side-only="true" xmlns="http://www.w3.org/1999/xhtml" xmlns:str="http://exslt.org/strings" xmlns:xxx="http://general_server.org/xmlnamespaces/dummyxsl/2006" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:server="http://general_server.org/xmlnamespaces/server/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:interface="http://general_server.org/xmlnamespaces/interface/2006" xmlns:object="http://general_server.org/xmlnamespaces/object/2006" xmlns:dyn="http://exslt.org/dynamic" name="controller" controller="true" version="1.0" extension-element-prefixes="server dyn">
  <!-- prevent encoding of the &amp; -->
  <xsl:output method="text" media-type="application/x-javascript" encoding="UTF-8" omit-xml-declaration="yes"/>

  <xsl:include xpath="../process_http_request"/>
  <!-- MI context specific HTTP handlers -->
  <xsl:include xpath="~JavaScript/HTTP"/>
  <xsl:include xpath="~CSSStylesheet/view"/> <!-- mixed with CSS sometimes -->

  <xsl:template match="object:Request/gs:HTTP" mode="basic_http_headers_client_cache">
    <xsl:if test="$gs_stage_live">
      <xsl:apply-templates select="." mode="basic_http_headers_client_cache_on"/>
    </xsl:if>
    <xsl:else>
      <xsl:apply-templates select="." mode="basic_http_headers_client_cache_off"/>
    </xsl:else>
  </xsl:template>  

  <xsl:template match="*|@*|text()" mode="gs_check_javascript_reserved_words">
    <xsl:param name="gs_name" select="@name"/>

    <!-- NOTE: this is not client side compatable!!!!
      we run this only on Server side because that is where the info is
      could also do it this way:
        <xsl:if test="contains(',abstract,arguments,boolean,break,byte,case,catch,char,class,const,continue,debugger,default,delete,do,double,else,enum,eval,export,extends,false,final,finally,float,for,function,goto,if,implements,import,in,instanceof,int,interface,let,long,native,new,null,package,private,protected,public,return,short,static,super,switch,synchronized,this,throw,throws,transient,true,try,typeof,var,void,volatile,while,with,yield,', $gs_comma_name)">Javascript reserved word</xsl:if>
        <xsl:if test="contains(',Array,Date,eval,function,hasOwnProperty,Infinity,isFinite,isNaN,isPrototypeOf,length,Math,NaN,name,Number,Object,prototype,String,toString,undefined,valueOf,', $gs_comma_name)">Javascript built-in object</xsl:if>
        <xsl:if test="contains(',getClass,java,JavaArray,javaClass,JavaObject,JavaPackage,', $gs_comma_name)">Java reserved word</xsl:if>
        <xsl:if test="contains(',alert,all,anchor,anchors,area,assign,blur,button,checkbox,clearInterval,clearTimeout,clientInformation,close,closed,confirm,constructor,crypto,decodeURI,decodeURIComponent,defaultStatus,document,element,elements,embed,embeds,encodeURI,encodeURIComponent,escape,event,fileUpload,focus,form,forms,frame,innerHeight,innerWidth,layer,layers,link,location,mimeTypes,navigate,navigator,frames,frameRate,hidden,history,image,images,offscreenBuffering,open,opener,option,outerHeight,outerWidth,packages,pageXOffset,pageYOffset,parent,parseFloat,parseInt,password,pkcs11,plugin,prompt,propertyIsEnum,radio,reset,screenX,screenY,scroll,secure,select,self,setInterval,setTimeout,status,submit,taint,text,textarea,top,unescape,untaint,window,', $gs_comma_name)">HTML window function</xsl:if>
        <xsl:if test="contains(',onblur,onclick,onerror,onfocus,onkeydown,onkeypress,onkeyup,onmouseover,onload,onmouseup,onmousedown,onsubmit,', $gs_comma_name)">Javascript HTML event</xsl:if>
    -->

    <xsl:variable name="gs_reserved_name" select="~JavaScript/javascript:reserved-names/*/*[name() = $gs_name]"/>

    <xsl:if test="$gs_reserved_name">
      <xsl:variable name="gs_reserved_name_group" select="$gs_reserved_name/.."/>

      <xsl:text>//name [</xsl:text><xsl:value-of select="$gs_name"/>] caused a [<xsl:value-of select="$gs_reserved_name_group/@message"/><xsl:text>] @message</xsl:text>
      <xsl:value-of select="$gs_newline"/>

      <xsl:if test="not($gs_reserved_name_group/@error-level = 'none')">
        <xsl:text>if (window.console) window.console.</xsl:text>
        <xsl:value-of select="$gs_reserved_name_group/@error-level"/>
        <xsl:if test="not($gs_reserved_name_group/@error-level)">error</xsl:if>
        <xsl:text>("[</xsl:text>
        <xsl:value-of select="$gs_name"/>
        <xsl:text>] caused [</xsl:text>
        <xsl:value-of select="$gs_reserved_name_group/@message"/>
        <xsl:text>]");</xsl:text>
        <xsl:value-of select="$gs_newline"/>
      </xsl:if>
    </xsl:if>
  </xsl:template>
</xsl:stylesheet>
