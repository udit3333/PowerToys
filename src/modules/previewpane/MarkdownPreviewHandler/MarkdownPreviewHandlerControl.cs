﻿// Copyright (c) Microsoft Corporation
// The Microsoft Corporation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System;
using System.IO;
using System.Text;
using System.Windows.Forms;
using Common;
using Markdig;
using Markdig.Helpers;

namespace MarkdownPreviewHandler
{
    /// <summary>
    /// Win Form Implementation for Markdown Preview Handler.
    /// </summary>
    public class MarkdownPreviewHandlerControl : FormHandlerControl
    {
        /// <summary>
        /// Extension to modify markdown AST.
        /// </summary>
        private readonly HTMLParsingExtension extension;

        /// <summary>
        /// Markdig Pipeline builder.
        /// </summary>
        private readonly MarkdownPipelineBuilder pipelineBuilder;

        /// <summary>
        /// Markdown HTML header.
        /// </summary>
        private readonly string htmlHeader = "<!doctype html><style>body{width:100%;margin:0;font-family:-apple-system,BlinkMacSystemFont,\"Segoe UI\",Roboto,\"Helvetica Neue\",Arial,\"Noto Sans\",sans-serif,\"Apple Color Emoji\",\"Segoe UI Emoji\",\"Segoe UI Symbol\",\"Noto Color Emoji\";font-size:1rem;font-weight:400;line-height:1.5;color:#212529;text-align:left;background-color:#fff}.container{padding:5%}body img{max-width:100%;height:auto}body h1,body h2,body h3,body h4,body h5,body h6{margin-top:24px;margin-bottom:16px;font-weight:600;line-height:1.25}body h1,body h2{padding-bottom:.3em;border-bottom:1px solid #eaecef}body{font-family:-apple-system,BlinkMacSystemFont,Segoe UI,Helvetica,Arial,sans-serif,Apple Color Emoji,Segoe UI Emoji}body h3{font-size:1.25em}body h4{font-size:1em}body h5{font-size:.875em}body h6{font-size:.85em;color:#6a737d}pre{font-family:SFMono-Regular,Consolas,Liberation Mono,Menlo,monospace;background-color:#f6f8fa;border-radius:3px;padding:16px;font-size:85%}a{color:#0366d6}strong{font-weight:600}em{font-style:italic}code{padding:.2em .4em;margin:0;font-size:85%;background-color:#f6f8fa;border-radius:3px}hr{border-color:#EEE -moz-use-text-color #FFF;border-style:solid none;border-width:.5px 0;margin:18px 0}table{display:block;width:100%;overflow:auto;border-spacing:0;border-collapse:collapse}tbody{display:table-row-group;vertical-align:middle;border-color:inherit;display:table-row;vertical-align:inherit;border-color:inherit}table tr{background-color:#fff;border-top:1px solid #c6cbd1}tr{display:table-row;vertical-align:inherit;border-color:inherit}table td,table th{padding:6px 13px;border:1px solid #dfe2e5}th{font-weight:600;display:table-cell;vertical-align:inherit;font-weight:bold;text-align:-internal-center}thead{display:table-header-group;vertical-align:middle;border-color:inherit}td{display:table-cell;vertical-align:inherit}code,pre,tt{font-family:SFMono-Regular,Menlo,Monaco,Consolas,\"Liberation Mono\",\"Courier New\",monospace;color:#24292e;overflow-x:auto}pre code{font-size:inherit;color:inherit;word-break:normal}blockquote{background-color:#fff;border-radius:3px;padding:15px;font-size:14px;display:block;margin-block-start:1em;margin-block-end:1em;margin-inline-start:40px;margin-inline-end:40px;padding:0 1em;color:#6a737d;border-left:.25em solid #dfe2e5}</style><body><div class=\"container\">";

        /// <summary>
        /// Markdown HTML footer.
        /// </summary>
        private readonly string htmlFooter = "</div></body></html>";

        /// <summary>
        /// Initializes a new instance of the <see cref="MarkdownPreviewHandlerControl"/> class.
        /// </summary>
        public MarkdownPreviewHandlerControl()
        {
            this.extension = new HTMLParsingExtension();
            this.pipelineBuilder = new MarkdownPipelineBuilder().UseAdvancedExtensions().UseEmojiAndSmiley();
            this.pipelineBuilder.Extensions.Add(this.extension);
        }

        /// <summary>
        /// Start the preview on the Control.
        /// </summary>
        /// <param name="dataSource">Path to the file.</param>
        public override void DoPreview<T>(T dataSource)
        {
            this.InvokeOnControlThread(() =>
            {
                StringBuilder sb = new StringBuilder();
                string filePath = dataSource as string;
                string fileText = File.ReadAllText(filePath);
                this.extension.BaseUrl = Path.GetDirectoryName(filePath);

                MarkdownPipeline pipeline = this.pipelineBuilder.Build();
                string parsedMarkdown = Markdown.ToHtml(fileText, pipeline);
                sb.AppendFormat("{0}{1}{2}", this.htmlHeader, parsedMarkdown, this.htmlFooter);

                WebBrowser browser = new WebBrowser();
                browser.DocumentText = sb.ToString();
                browser.Dock = DockStyle.Fill;
                browser.IsWebBrowserContextMenuEnabled = false;
                browser.ScriptErrorsSuppressed = true;
                browser.ScrollBarsEnabled = true;
                this.Controls.Add(browser);
                base.DoPreview(dataSource);
            });
        }
    }
}
