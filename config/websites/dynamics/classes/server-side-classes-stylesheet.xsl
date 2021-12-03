<xsl:stylesheet xmlns="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:str="http://exslt.org/strings" xmlns:response="http://general_server.org/xmlnamespaces/response/2006" xmlns:server="http://general_server.org/xmlnamespaces/server/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:class="http://general_server.org/xmlnamespaces/class/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:dyn="http://exslt.org/dynamic" name="server-side-classes-stylesheet" version="1.0" extension-element-prefixes="dyn">
  <!-- we DO NOT use context dependent $gs_global_variables here
    because this will be used by TXml without context
    TODO: TXml could save the global variable stack also
    e.g. <gs_website_root>/repository:websites/object:Website[@name='general_server']</gs_website_root>
  -->
  <xsl:include xpath="top::class:*/descendant-natural::xsl:stylesheet[str:boolean(@response:server-side-only)]"/>
</xsl:stylesheet>
