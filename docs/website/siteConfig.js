/**
 * Copyright (c) 2017-present, Facebook, Inc.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// See https://docusaurus.io/docs/site-config for all the possible
// site configuration options.

// List of projects/orgs using your project for the users page.
//const users = [
//  {
//    caption: 'Chris Mc',
    // You will need to prepend the image path with your baseUrl
    // if it is not '/', like: '/test-site/img/docusaurus.svg'.
//    image: 'https://avatars3.githubusercontent.com/u/16867443?s=460&v=4',
//    infoLink: 'https://www.github.com/prince-chrismc',
//    pinned: true,
//  },
//];

const siteConfig = {
  title: 'Simple Socket', // Title for your website.
  tagline: 'Simple Transport Layer Networking',
  url: 'https://prince-chrismc.github.com', // Your website URL
  baseUrl: '/Simple-Socket/', // Base URL for your project */
  // For github.io type URLs, you would set the url and baseUrl like:
  //   url: 'https://facebook.github.io',
  //   baseUrl: '/test-site/',

  // Used for publishing and more
  projectName: 'Simple-Socket',
  organizationName: 'prince-chrismc',
  // For top-level user or org sites, the organization is still the same.
  // e.g., for the https://JoelMarcey.github.io site, it would be set like...
  //   organizationName: 'JoelMarcey'

  // For no header links in the top nav bar -> headerLinks: [],
  headerLinks: [
    {doc: 'Intro', label: 'Intro'},
    {doc: 'Getting-Started', label: 'Getting Started'},
    {doc: 'CSimpleSocket', label: 'API'},
    { href: 'https://github.com/prince-chrismc/Simple-Socket', label: 'GitHub' },
    {page: 'help', label: 'Help'},
    //{blog: true, label: 'Blog'},
    //{ search: true },
  ],

  // If you have users set above, you add it here:
  //users,

  /* path to images for header/footer */
  headerIcon: 'img/rj45-512.gif',
  footerIcon: 'img/rj45-512.gif',
  favicon: 'img/favicon/favicon.ico',

  /* Colors for website */
  colors: {
    primaryColor: '#333333',
    secondaryColor: '#2d5b84',
  },

  /* Custom fonts for website */
  /*
  fonts: {
    myFont: [
      "Times New Roman",
      "Serif"
    ],
    myOtherFont: [
      "-apple-system",
      "system-ui"
    ]
  },
  */

  // This copyright info is used in /core/Footer.js and blog RSS/Atom feeds.
  copyright: `Copyright © ${new Date().getFullYear()} Christopher McArthur`,

  highlight: {
    // Highlight.js theme to use for syntax highlighting in code blocks.
    theme: 'default',
  },

  // Add custom scripts here that would be placed in <script> tags.
  scripts: ['https://buttons.github.io/buttons.js'],

  // On page navigation for the current documentation page.
  onPageNav: 'separate',
  // No .html extensions for paths.
  cleanUrl: true,

  // You may provide arbitrary config keys to be used as needed by your
  // template. For example, if you need your repo's URL...
  //   repoUrl: 'https://github.com/facebook/test-site',
};

module.exports = siteConfig;
