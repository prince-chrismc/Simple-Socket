/**
 * Copyright (c) 2017-present, Facebook, Inc.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

const React = require('react');

const CompLibrary = require('../../core/CompLibrary.js');

const MarkdownBlock = CompLibrary.MarkdownBlock; /* Used to read markdown */
const Container = CompLibrary.Container;
const GridBlock = CompLibrary.GridBlock;

const siteConfig = require(`${process.cwd()}/siteConfig.js`);

function imgUrl(img) {
  return `${siteConfig.baseUrl}img/${img}`;
}

function docUrl(doc, language) {
  return `${siteConfig.baseUrl}docs/${language ? `${language}/` : ''}${doc}`;
}

function pageUrl(page, language) {
  return siteConfig.baseUrl + (language ? `${language}/` : '') + page;
}

class Button extends React.Component {
  render() {
    return (
      <div className="pluginWrapper buttonWrapper">
        <a className="button" href={this.props.href} target={this.props.target}>
          {this.props.children}
        </a>
      </div>
    );
  }
}

Button.defaultProps = {
  target: '_self',
};

const SplashContainer = props => (
  <div className="homeContainer">
    <div className="homeSplashFade">
      <div className="wrapper homeWrapper">{props.children}</div>
    </div>
  </div>
);

const Logo = props => (
  <div className="projectLogo">
    <img src={props.img_src} alt="Project Logo" />
  </div>
);

const ProjectTitle = () => (
  <h2 className="projectTitle">
    {siteConfig.title}
    <small>{siteConfig.tagline}</small>
  </h2>
);

const PromoSection = props => (
  <div className="section promoSection">
    <div className="promoRow">
      <div className="pluginRowBlock">{props.children}</div>
    </div>
  </div>
);

class HomeSplash extends React.Component {
  render() {
    const language = this.props.language || '';
    return (
      <SplashContainer>
        <Logo img_src={imgUrl('rj45-256.png')} />
        <div className="inner">
          <ProjectTitle />
          <PromoSection>
            <Button href="#try">Try It Out</Button>
            <Button href={docUrl('Intro', language)}>Overview</Button>
            <Button href={docUrl('Getting-Started', language)}>Get Started</Button>
          </PromoSection>
        </div>
      </SplashContainer>
    );
  }
}

const Block = props => (
  <Container
    padding={['bottom', 'top']}
    id={props.id}
    background={props.background}>
    <GridBlock align="center" contents={props.children} layout={props.layout} />
  </Container>
);

const FeatureCallout = () => (
  <div
    className="productShowcaseSection paddingBottom"
    style={{textAlign: 'center'}}>
    <MarkdownBlock>This ~~fork~~ repository aims to have the original library compile and work reliably using modern c++ compilers ( MSVC 15.7 / GCC 7.3 / Clang 6.0 ) with a focus on
the async and multicast functionality.</MarkdownBlock>
  </div>
);

const Features = () => (
  <Block layout="threeColumn">
    {[
      {
        content: 'Application implmentation shouldn\'t be limited by the underlying protocol. You can easily change the type with one parameter when instantiating your socket!',
        image: imgUrl('rj45-256.png'),
        imageAlign: 'top',
        title: 'Protocol Independent',
      },
      {
        content: 'Provides a minimalistic approach to non-blocking socket I/O. Requires the application to poll instead of using callbacks.',
        image: imgUrl('rj45-256.png'),
        imageAlign: 'top',
        title: 'Asynchronous',
      },
      {
        content: 'Offers IGMPv2 support when using UDP sockets. Easily multicast your "Hello World" message to any group.',
        image: imgUrl('rj45-256.png'),
        imageAlign: 'top',
        title: 'Multicast Support',
      },
    ]}
  </Block>
);

const LearnHow = () => (
  <Block id="try" background="light">
    {[
      {
        content: 'Talk about learning how to use this',
        image: imgUrl('rj45-256.png'),
        imageAlign: 'right',
        title: 'Learn How',
      },
    ]}
  </Block>
);

class Index extends React.Component {
  render() {
    const language = this.props.language || '';

    return (
      <div>
        <HomeSplash language={language} />
        <div className="mainContainer">
          <FeatureCallout />
          <Features />
          <LearnHow />
        </div>
      </div>
    );
  }
}

module.exports = Index;
