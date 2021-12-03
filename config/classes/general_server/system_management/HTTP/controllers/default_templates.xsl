<xsl:stylesheet response:server-side-only="true" xmlns="http://www.w3.org/1999/xhtml" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:server="http://general_server.org/xmlnamespaces/server/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:dyn="http://exslt.org/dynamic" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:interface="http://general_server.org/xmlnamespaces/interface/2006" xmlns:object="http://general_server.org/xmlnamespaces/object/2006" name="default_templates" version="1.0" extension-element-prefixes="dyn database server debug">
  <!-- prevent default traversal of nodes without a mode
    whilst this is not dangerous in itself
    a transform of /object:Server without mode would assemble the text() from the entire database
  -->
  <xsl:template match="*">
    <debug:server-message output="traversal of node [{name()}/{@xml:id}] without mode attempted" type="warning"/>
  </xsl:template>
</xsl:stylesheet>
