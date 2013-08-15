#include "ndnx-selectors.h"
#include "ndnx-common.h"
#include <boost/lexical_cast.hpp>

using namespace std;

namespace Ndnx {

Selectors::Selectors()
          : m_maxSuffixComps(-1)
          , m_minSuffixComps(-1)
          , m_answerOriginKind(AOK_DEFAULT)
          , m_interestLifetime(-1.0)
          , m_scope(NO_SCOPE)
          , m_childSelector(DEFAULT)
{
}

Selectors::Selectors(const Selectors &other)
{
  m_maxSuffixComps = other.m_maxSuffixComps;
  m_minSuffixComps = other.m_minSuffixComps;
  m_answerOriginKind = other.m_answerOriginKind;
  m_interestLifetime = other.m_interestLifetime;
  m_scope = other.m_scope;
  m_childSelector = other.m_childSelector;
  m_publisherPublicKeyDigest = other.m_publisherPublicKeyDigest;
}

Selectors::Selectors(const ndn_parsed_interest *pi)
          : m_maxSuffixComps(-1)
          , m_minSuffixComps(-1)
          , m_answerOriginKind(AOK_DEFAULT)
          , m_interestLifetime(-1.0)
          , m_scope(NO_SCOPE)
          , m_childSelector(DEFAULT)
{
  if (pi != NULL)
  {
    m_maxSuffixComps = pi->max_suffix_comps;
    m_minSuffixComps = pi->min_suffix_comps;
    switch(pi->orderpref)
    {
      case 0: m_childSelector = LEFT; break;
      case 1: m_childSelector = RIGHT; break;
      default: break;
    }
    switch(pi->answerfrom)
    {
      case 0x1: m_answerOriginKind = AOK_CS; break;
      case 0x2: m_answerOriginKind = AOK_NEW; break;
      case 0x3: m_answerOriginKind = AOK_DEFAULT; break;
      case 0x4: m_answerOriginKind = AOK_STALE; break;
      case 0x10: m_answerOriginKind = AOK_EXPIRE; break;
      default: break;
    }
    m_scope = static_cast<Scope> (pi->scope);
    // scope and interest lifetime do not really matter to receiving application, it's only meaningful to routers
  }
}

bool
Selectors::operator == (const Selectors &other)
{
  return m_maxSuffixComps == other.m_maxSuffixComps
         && m_minSuffixComps == other.m_minSuffixComps
         && m_answerOriginKind == other.m_answerOriginKind
         && (m_interestLifetime - other.m_interestLifetime) < 10e-4
         && m_scope == other.m_scope
         && m_childSelector == other.m_childSelector;
}

bool
Selectors::isEmpty() const
{
  return m_maxSuffixComps == -1
         && m_minSuffixComps == -1
         && m_answerOriginKind == AOK_DEFAULT
         && (m_interestLifetime - (-1.0)) < 10e-4
         && m_scope == NO_SCOPE
         && m_childSelector == DEFAULT;
}


NdnxCharbufPtr
Selectors::toNdnxCharbuf() const
{
  if (isEmpty())
  {
    return NdnxCharbufPtr ();
  }
  NdnxCharbufPtr ptr(new NdnxCharbuf());
  ndn_charbuf *cbuf = ptr->getBuf();
  ndn_charbuf_append_tt(cbuf, NDN_DTAG_Interest, NDN_DTAG);
  ndn_charbuf_append_tt(cbuf, NDN_DTAG_Name, NDN_DTAG);
  ndn_charbuf_append_closer(cbuf); // </Name>

  if (m_maxSuffixComps < m_minSuffixComps)
  {
    boost::throw_exception(InterestSelectorException() << error_info_str("MaxSuffixComps = " + boost::lexical_cast<string>(m_maxSuffixComps) + " is smaller than  MinSuffixComps = " + boost::lexical_cast<string>(m_minSuffixComps)));
  }

  if (m_minSuffixComps > 0)
  {
    ndnb_tagged_putf(cbuf, NDN_DTAG_MinSuffixComponents, "%d", m_minSuffixComps);
  }

  if (m_maxSuffixComps > 0)
  {
    ndnb_tagged_putf(cbuf, NDN_DTAG_MaxSuffixComponents, "%d", m_maxSuffixComps);
  }

  // publisher digest

  // exclude

  if (m_childSelector != DEFAULT)
  {
    ndnb_tagged_putf(cbuf, NDN_DTAG_MinSuffixComponents, "%d", (int)m_minSuffixComps);
  }

  if (m_answerOriginKind != AOK_DEFAULT)
  {
    // it was not using "ndnb_tagged_putf" in ndnx c code, no idea why
    ndn_charbuf_append_tt(cbuf, NDN_DTAG_AnswerOriginKind, NDN_DTAG);
    ndnb_append_number(cbuf, m_answerOriginKind);
    ndn_charbuf_append_closer(cbuf); // <AnswerOriginKind>
  }

  if (m_scope != NO_SCOPE)
  {
    ndnb_tagged_putf(cbuf, NDN_DTAG_Scope, "%d", m_scope);
  }

  if (m_interestLifetime > 0.0)
  {
    // Ndnx timestamp unit is weird 1/4096 second
    // this is from their code
    unsigned lifetime = 4096 * (m_interestLifetime + 1.0/8192.0);
    if (lifetime == 0 || lifetime > (30 << 12))
    {
      boost::throw_exception(InterestSelectorException() << error_info_str("Ndnx requires 0 < lifetime < 30.0. lifetime= " + boost::lexical_cast<string>(m_interestLifetime)));
    }
    unsigned char buf[3] = {0};
    for (int i = sizeof(buf) - 1; i >= 0; i--, lifetime >>= 8)
    {
      buf[i] = lifetime & 0xff;
    }
    ndnb_append_tagged_blob(cbuf, NDN_DTAG_InterestLifetime, buf, sizeof(buf));
  }

  ndn_charbuf_append_closer(cbuf); // </Interest>

  return ptr;
}

} // Ndnx
