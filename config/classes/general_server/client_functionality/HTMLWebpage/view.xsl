<xsl:stylesheet xmlns:str="http://exslt.org/strings" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://www.w3.org/1999/xhtml" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" name="view" version="1.0">
  <!-- ################################# normal webpage ################################# -->
  <xsl:template match="interface:HTMLWebpage" meta:interface-template="yes">
    <xsl:param name="gs_html_identifier_class"/>
    
    <!-- note that this allows many meta:environment on any ancestor -->
    <xsl:variable name="gs_environments" select="ancestor::*/meta:environment"/>

    <!-- checks -->
    <xsl:if test="not($gs_resource_server)"><xsl:comment>no $gs_resource_server: webpage @components libraries will be loaded from this same server</xsl:comment></xsl:if>
    <xsl:if test="html:head"><xsl:comment>html:head tag not relevant in interface:HTMLWebpage. you should use meta:head instead</xsl:comment></xsl:if>
    <xsl:if test="not(@general-server-framework)"><xsl:comment>the general server framework has not been included. nothing will work!</xsl:comment></xsl:if>
    <xsl:if test="not(@code-class)"><xsl:comment>class JavaScript has not been included. the objects will have no functionality</xsl:comment></xsl:if>
    <xsl:if test="not(@class-styles)"><xsl:comment>class CSS styles has not been included. bare non-formatted output</xsl:comment></xsl:if>
    <xsl:if test="not($gs_environments)"><xsl:comment>no meta:environment discovered (Server, LinkedServer, User, etc.)</xsl:comment></xsl:if>

    <html>
      <head>
        <xsl:copy-of select="meta:head/@*"/>

        <!-- controlled meta -->
        <meta name="Copyright" content="Annesley Solutions ltd. All rights reserved"/>
        <meta name="Resource-Server" content="{$gs_resource_server}"/>
        <meta charset="UTF-8"/>

        <!-- default html:meta -->
        <xsl:if test="not(meta:head/html:meta/@name='robots')"><meta name="robots" content=""/></xsl:if>
        <xsl:if test="not(meta:head/html:meta/@name='revisit-after')"><meta name="revisit-after" content="2"/></xsl:if>
        <xsl:if test="not(meta:head/html:meta/@name='reply-to')"><meta name="reply-to" content="annesley_newholm [at] yahoo [dot] it"/></xsl:if>
        <xsl:if test="not(meta:head/html:meta/@name='Rating')"><meta name="Rating" content="General"/></xsl:if>
        <xsl:if test="not(meta:head/html:meta/@name='Pragma')"><meta name="Pragma" content="no-cache"/></xsl:if>
        <xsl:if test="not(meta:head/html:meta/@name='Language')"><meta name="Language" content="en"/></xsl:if>
        <xsl:if test="not(meta:head/html:meta/@name='distribution')"><meta name="distribution" content="Global"/></xsl:if>
        <xsl:if test="not(meta:head/html:meta/@name='Classification')"><meta name="Classification" content="Research"/></xsl:if>
        <xsl:if test="not(meta:head/html:meta/@name='Author')"><meta name="Author" content="Annesley Newholm"/></xsl:if>

        <xsl:comment>webpage @components (<xsl:value-of select="count(@*)"/>)</xsl:comment>
        <xsl:apply-templates select="@*" mode="gs_view_render_head"/>

        <!-- Server, User, Database, Request, ... -->
        <xsl:if test="$gs_environments">
          <xsl:comment>environment objects (<xsl:value-of select="count($gs_environments/*)"/>)</xsl:comment>
          <xsl:apply-templates select="$gs_environments/*" mode="gs_view_render"/>
        </xsl:if>

        <!-- custom meta info, extra libraries -->
        <xsl:apply-templates select="meta:head/*" mode="gs_view_render"/>
      </head>

      <body id="body">
        <xsl:attribute name="class">
          <xsl:text>body </xsl:text>
          <xsl:if test="@body-class"><xsl:value-of select="@body-class"/><xsl:text> </xsl:text></xsl:if>
          <xsl:value-of select="$gs_html_identifier_class"/>
        </xsl:attribute>

        <xsl:apply-templates mode="gs_view_render"/>
        <div id="gs-copyright">© Annesley Solutions Ltd. 2013 All rights reserved.</div>
      </body>
    </html>
  </xsl:template>

  <xsl:template match="interface:HTMLWebpage/html:head"/>

  <xsl:template match="interface:HTMLWebpage/html:head/html:meta[@name='Copyright']">
    <xsl:comment>html:meta tag [<xsl:value-of select="@name"/>] not allowed</xsl:comment>
  </xsl:template>

  <xsl:template match="interface:HTMLWebpage/html:head/html:meta[@charset]">
    <xsl:comment>html:meta tag [@charset] not allowed</xsl:comment>
  </xsl:template>

  <!-- ################################# gs_view_render_head ################################# -->
  <xsl:template match="interface:HTMLWebpage/@*" mode="gs_view_render_head"/>

  <xsl:template match="interface:HTMLWebpage/@response:classes" mode="gs_view_render_head">
    <xsl:comment><xsl:value-of select="."/></xsl:comment>
  </xsl:template>
  
  <xsl:template match="interface:HTMLWebpage/@title" mode="gs_view_render_head">
    <title><xsl:value-of select="."/></title>
  </xsl:template>

  <xsl:template match="interface:HTMLWebpage/@favicon" mode="gs_view_render_head">
    <link rel="shortcut icon" href="{.}"></link>
    <link rel="icon" href="{.}" type="image/x-icon"></link>    
  </xsl:template>

  <xsl:template match="interface:HTMLWebpage/@jQuery" mode="gs_view_render_head">
    <xsl:apply-templates select="." mode="gs_view_render_libraries">
      <xsl:with-param name="gs_standard_libraries" select="'jquery'"/>
      <xsl:with-param name="gs_min" select="../@jQuery-delivery"/>
      <xsl:with-param name="gs_version" select="."/>
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="interface:HTMLWebpage/@jQueryUI" mode="gs_view_render_head">
    <xsl:apply-templates select="." mode="gs_view_render_libraries">
      <xsl:with-param name="gs_standard_libraries" select="'jquery-ui'"/>
      <xsl:with-param name="gs_min" select="../@jQuery-delivery"/>
      <xsl:with-param name="gs_version" select="."/>
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="interface:HTMLWebpage/@standard-libraries" mode="gs_view_render_head">
    <xsl:apply-templates select="." mode="gs_view_render_libraries"/>
  </xsl:template>

  <xsl:template match="interface:HTMLWebpage/@general-server-framework" mode="gs_view_render_head">
    <script type="text/javascript" src="/~JavaScript/repository:framework/javascript:code.js">
      <xsl:apply-templates select="." mode="gs_enforce_non_self_closing"/>
    </script>
    <!-- script type="text/javascript" src="/~JavaScript/repository:framework/repository:classes/descendant-natural::class:*.js"/ -->
  </xsl:template>

  <xsl:template match="interface:HTMLWebpage/@class-styles" mode="gs_view_render_head">
    <!-- Classes CSS. one file for all: efficient but difficult to auto-reload changes -->
    <!-- TODO: style rel="stylesheet" type="text/css" href="/$gs_website_classes/css:stylesheet/css:dependency"/
      this was an xsl:apply-templates in thexsl:stylesheet transform
    -->
    <xsl:variable name="gs_classes_and_bases" select="string(../@response:classes-and-bases)"/>

    <link rel="stylesheet" type="text/css">
      <xsl:attribute name="href">
        <xsl:text>/</xsl:text>
        <xsl:if test="$gs_classes_and_bases">(<xsl:value-of select="$gs_classes_and_bases"/>)</xsl:if>
        <xsl:if test="not($gs_classes_and_bases)">$gs_website_classes</xsl:if>
        <xsl:text>/css:</xsl:text>
        <xsl:text>stylesheet.css</xsl:text>
      </xsl:attribute>
    </link>
  </xsl:template>

  <xsl:template match="interface:HTMLWebpage/@code-class" mode="gs_view_render_head">
    <!-- Classes JavaScript. one file for all: efficient but difficult to auto-reload changes -->
    <!-- TODO: script type="text/javascript" src="/$gs_website_classes/javascript:code/javascript:dependency"/
      this was an xsl:apply-templates in the xsl:stylesheet transform
    -->
    <xsl:variable name="gs_classes_and_bases" select="string(../@response:classes-and-bases)"/>
    
    <script type="text/javascript">
      <xsl:attribute name="src">
        <xsl:text>/</xsl:text>
        <xsl:if test="$gs_classes_and_bases">(<xsl:value-of select="$gs_classes_and_bases"/>)</xsl:if>
        <xsl:if test="not($gs_classes_and_bases)">$gs_website_classes</xsl:if>
        <xsl:text>.js</xsl:text>
      </xsl:attribute>

      <xsl:apply-templates select="." mode="gs_enforce_non_self_closing"/>
    </script>
  </xsl:template>

  <xsl:template match="interface:HTMLWebpage/@code-class-dependencies" mode="gs_view_render_head">
    <xsl:apply-templates select="." mode="gs_external_class_javascript_dependencies"/>
  </xsl:template>

  <xsl:template match="interface:HTMLWebpage/@class-style-dependencies" mode="gs_view_render_head">
    <xsl:apply-templates select="." mode="gs_external_class_css_dependencies"/>
  </xsl:template>

  <!-- ################################# 3rd party JS libraries ################################# -->
  <xsl:template match="*|node()|@*|text()" mode="gs_view_render_libraries">
    <xsl:param name="gs_standard_libraries" select="string(.)"/>
    <xsl:param name="gs_version"/>
    <xsl:param name="gs_minified"/>
    
    <xsl:variable name="gs_standard_library_fragment">
      <xsl:if test="contains($gs_standard_libraries, ',')"><xsl:value-of select="substring-before($gs_standard_libraries, ',')"/></xsl:if>
      <xsl:if test="not(contains($gs_standard_libraries, ','))"><xsl:value-of select="$gs_standard_libraries"/></xsl:if>
    </xsl:variable>
    <xsl:variable name="gs_standard_library" select="string($gs_standard_library_fragment)"/>

    <!-- e.g. 
      modenizr      => http://resources-server.com/resources/shared/modenizr.js
      jquery.cookie => http://resources-server.com/resources/shared/jquery/plugins/modenizr.js 
    -->
    <script type="text/javascript">
      <xsl:attribute name="src">
        <xsl:value-of select="$gs_resource_server"/>
        <xsl:text>/resources/shared/</xsl:text>
        <xsl:if test="starts-with($gs_standard_library, 'jquery')">jquery/</xsl:if>
        <xsl:if test="starts-with($gs_standard_library, 'jquery.')">plugins/</xsl:if>
        <xsl:value-of select="$gs_standard_library"/>
        <xsl:if test="$gs_version">-<xsl:value-of select="$gs_version"/></xsl:if>
        <xsl:if test="$gs_minified">.min</xsl:if>
        <xsl:text>.js</xsl:text>
      </xsl:attribute>  
      
      <!-- checks -->
      <xsl:if test="not($gs_resource_server)"><xsl:attribute name="gs:warning">including [<xsl:value-of select="$gs_standard_library"/>] from same server because $gs_resource_server not present</xsl:attribute></xsl:if>

      <xsl:apply-templates select="." mode="gs_enforce_non_self_closing"/>
    </script>
    
    <xsl:if test="contains($gs_standard_libraries, ',')">
      <xsl:apply-templates select="." mode="gs_view_render_libraries">
        <xsl:with-param name="gs_standard_libraries" select="substring-after($gs_standard_libraries, ',')"/>
      </xsl:apply-templates>
    </xsl:if>
  </xsl:template>
  
  <!-- ################################# editor ################################# -->
  <xsl:template match="html:html" mode="editor" meta:interface-template="yes">
    <xsl:apply-templates select="."/>
  </xsl:template>

  <xsl:template match="html:html" mode="default_content">
    <!-- default interface leads to render default_content -->
    <xsl:apply-templates select="*" mode="gs_view_render"/>
  </xsl:template>

  <!-- ################################# interface_render? ################################# -->
  <!-- xsl:template match="html:link" mode="gs_interface_render">
    <div class="gs-warning">embedded webpage link to [<xsl:value-of select="@href"/>] will not work</div>
  </xsl:template>

  <xsl:template match="html:script" mode="gs_interface_render">
    <div class="gs-warning">
      <xsl:if test="@src">external linked webpage script to [<xsl:value-of select="@src"/>] will not work</xsl:if>
      <xsl:if test="not(@src)">embedded script suppressed</xsl:if>
    </div>
  </xsl:template>

  <xsl:template match="html:meta" mode="gs_interface_render"/>

  <xsl:template match="html:title" mode="gs_interface_render"/>

  <xsl:template match="html:body" mode="gs_interface_render">
    <div>
      <xsl:copy-of select="@*[not(.=../@class)]"/>
      <xsl:attribute name="class">
        <xsl:value-of select="@class"/>
        <xsl:text> gs-html-fake-body-div</xsl:text>
      </xsl:attribute>
      <xsl:apply-templates select="*|text()" mode="gs_interface_render"/>
    </div>
  </xsl:template -->
</xsl:stylesheet>
