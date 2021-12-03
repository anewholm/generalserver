<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://www.w3.org/1999/xhtml" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:interface="http://general_server.org/xmlnamespaces/interface/2006" xmlns:object="http://general_server.org/xmlnamespaces/object/2006" name="view" version="1.0">
  <xsl:template match="interface:IDE/@title" mode="full_title"/>

  <xsl:template match="interface:IDE" mode="gs_doxygen_support">
    <!-- TODO: doxygen HTML support -->
    <!-- link href="{$gs_resource_server}/documentation/html/tabs.css" rel="stylesheet" type="text/css"/ -->
    <link href="{$gs_resource_server}/documentation/html/doxygen.css" rel="stylesheet" type="text/css"></link>
    <link href="{$gs_resource_server}/documentation/html/search/search.css" rel="stylesheet" type="text/css"></link>
    <!-- script type="text/javascript" src="{$gs_doxygen_html_root}/search/search.js"/ -->
    <style>
      .gs-html-fake-body-div .gs-warning {display:none;}
      .gs-html-fake-body-div #top {display:none;}
    </style>
  </xsl:template>

  <xsl:template match="interface:IDE//xsd:schema[@name='search']" mode="gs_form_render_submit">
    <!-- multiple search options -->
    <div id="gs-submit-options" class="gs-search-option">
      <input id="gs-submit-text" class="gs-form-submit gs-first gs-selected" type="submit" name="gs--submit" value="text"/>
      <input id="gs-submit-xpath" class="gs-form-submit" type="submit" name="gs--submit" value="xpath"/>
      <input id="gs-submit-class" class="gs-form-submit" type="submit" name="gs--submit" value="class"/>
      <input id="gs-submit-xmlid" class="gs-form-submit gs-last" type="submit" name="gs--submit" value="xml:id"/>
    </div>
  </xsl:template>
</xsl:stylesheet>
