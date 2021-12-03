<xsl:stylesheet response:server-side-only="true" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:server="http://general_server.org/xmlnamespaces/server/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:interface="http://general_server.org/xmlnamespaces/interface/2006" xmlns:object="http://general_server.org/xmlnamespaces/object/2006" xmlns:str="http://exslt.org/strings" name="virtualhosts-classes" version="1.0" extension-element-prefixes="debug dyn str regexp database server request">
  <!-- classes info
    TODO: PERFORMANCE: will be cached in paths.xml eventually
    note how we avoid the inherited class:* and its xsl:stylesheets (class:*/class:*/.../xsl:stylesheet) 
    we demand that class:* xsl:stylesheets must either: be in the class:* root, or in a sub repository:*
  -->
  <xsl:param name="gs_website_repository_classes" select="$gs_website_root/repository:classes"/>
  <xsl:param name="gs_website_top_classes" select="$gs_website_repository_classes/top::class:*"/>
  <xsl:param name="gs_website_classes" select="$gs_website_top_classes|$gs_website_top_classes/descendant-natural::class:*"/>
  <!-- xsl:param name="gs_website_classes" select="$gs_website_root/repository:classes/top::class:*"/ -->
  <!-- avoid the inheritance (class:*/class:*/xsl:stylesheet) again when getting the xsl:stylesheets in sub-repositories -->
  <xsl:param name="gs_all_class_stylesheets" select="$gs_website_classes/xsl:stylesheet|$gs_website_classes/repository:*/xsl:stylesheet"/>
  <!-- sub-repositories can also be response:server-side-only so we need to check ancestors -->
  <xsl:param name="gs_client_side_class_stylesheets" select="$gs_all_class_stylesheets[not(str:boolean(@response:server-side-only) or ancestor::repository:*[str:boolean(@response:server-side-only)])]"/>
  <!-- xsl:stylesheet with one xpath xsl:include for all server-side class stylesheets -->
  <xsl:param name="gs_stylesheet_server_side_classes" select="$gs_website_repository_classes/server-side-classes-stylesheet"/>
</xsl:stylesheet>
