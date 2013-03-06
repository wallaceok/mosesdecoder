#include <stdexcept>

#include "util/check.hh"

#include "ChartHypothesis.h"
#include "ChartManager.h"
#include "FeatureFunction.h"
#include "Hypothesis.h"
#include "Manager.h"
#include "TranslationOption.h"

using namespace std;

namespace Moses
{

PhraseBasedFeatureContext::PhraseBasedFeatureContext(const Hypothesis* hypothesis) :
  m_hypothesis(hypothesis),
  m_translationOption(m_hypothesis->GetTranslationOption()),
  m_source(m_hypothesis->GetManager().GetSource()) {}

PhraseBasedFeatureContext::PhraseBasedFeatureContext
      (const TranslationOption& translationOption, const InputType& source) :
  m_hypothesis(NULL),
  m_translationOption(translationOption),
  m_source(source) {}

const TranslationOption& PhraseBasedFeatureContext::GetTranslationOption() const
{
  return m_translationOption;
}

const InputType& PhraseBasedFeatureContext::GetSource() const 
{
  return m_source;
}

const TargetPhrase& PhraseBasedFeatureContext::GetTargetPhrase() const
{
  return m_translationOption.GetTargetPhrase();
}

const WordsBitmap& PhraseBasedFeatureContext::GetWordsBitmap() const
{
  if (!m_hypothesis) {
    throw std::logic_error("Coverage vector not available during pre-calculation");
  } 
  return m_hypothesis->GetWordsBitmap();
}


ChartBasedFeatureContext::ChartBasedFeatureContext
                        (const ChartHypothesis* hypothesis):
  m_hypothesis(hypothesis),
  m_targetPhrase(hypothesis->GetCurrTargetPhrase()),
  m_source(hypothesis->GetManager().GetSource()) {}

ChartBasedFeatureContext::ChartBasedFeatureContext(
                         const TargetPhrase& targetPhrase,
                         const InputType& source):
  m_hypothesis(NULL),
  m_targetPhrase(targetPhrase),
  m_source(source) {}

const InputType& ChartBasedFeatureContext::GetSource() const
{
  return m_source;
}

const TargetPhrase& ChartBasedFeatureContext::GetTargetPhrase() const
{
  return m_targetPhrase;
}

multiset<string> FeatureFunction::description_counts;
const size_t FeatureFunction::unlimited = -1;

std::vector<FeatureFunction*> FeatureFunction::m_producers;
std::vector<const StatelessFeatureFunction*> StatelessFeatureFunction::m_statelessFFs;
std::vector<const StatefulFeatureFunction*>  StatefulFeatureFunction::m_statefulFFs;

FeatureFunction::FeatureFunction(const std::string& description, const std::string &line)
: m_reportSparseFeatures(false)
{
  ParseLine(description, line);
  m_numScoreComponents = FindNumFeatures();

  bool customName = FindName();
  if (!customName) {
    size_t index = description_counts.count(description);

    ostringstream dstream;
    dstream << description;
    dstream << index;

    description_counts.insert(description);
    m_description = dstream.str();
  }

  if (m_numScoreComponents != unlimited) {
	ScoreComponentCollection::RegisterScoreProducer(this);
  }

  m_producers.push_back(this);
}

FeatureFunction::FeatureFunction(const std::string& description, size_t numScoreComponents, const std::string &line)
: m_reportSparseFeatures(false), m_numScoreComponents(numScoreComponents)
{
  ParseLine(description, line);
  bool customName = FindName();
  if (!customName) {
    size_t index = description_counts.count(description);

    ostringstream dstream;
    dstream << description;
    dstream << index;

    description_counts.insert(description);
    m_description = dstream.str();
  }

  if (numScoreComponents != unlimited) {
    ScoreComponentCollection::RegisterScoreProducer(this);
  }

  m_producers.push_back(this);
}


FeatureFunction::~FeatureFunction() {}

void FeatureFunction::ParseLine(const std::string& description, const std::string &line)
{
  cerr << "line=" << line << endl;
  vector<string> toks = Tokenize(line);

  CHECK(toks.size());
  //CHECK(toks[0] == description);

  for (size_t i = 1; i < toks.size(); ++i) {
    vector<string> args = Tokenize(toks[i], "=");
    CHECK(args.size() == 2);
    m_args.push_back(args);
  }
}

size_t FeatureFunction::FindNumFeatures()
{
  for (size_t i = 0; i < m_args.size(); ++i) {
    if (m_args[i][0] == "num-features") {
      size_t ret = Scan<size_t>(m_args[i][1]);
      m_args.erase(m_args.begin() + i);
      return ret;
    }
  }
  CHECK(false);
}

bool FeatureFunction::FindName()
{
  for (size_t i = 0; i < m_args.size(); ++i) {
    if (m_args[i][0] == "name") {
      m_description = m_args[i][1];
      m_args.erase(m_args.begin() + i);
      return true;
    }
  }
  return false;
}

StatelessFeatureFunction::StatelessFeatureFunction(const std::string& description, const std::string &line)
:FeatureFunction(description, line)
{
  m_statelessFFs.push_back(this);
}

StatelessFeatureFunction::StatelessFeatureFunction(const std::string& description, size_t numScoreComponents, const std::string &line)
:FeatureFunction(description, numScoreComponents, line)
{
  m_statelessFFs.push_back(this);
}

bool StatelessFeatureFunction::IsStateless() const
{
  return true;
}

bool StatelessFeatureFunction::ComputeValueInTranslationOption() const
{
  return false;
}

StatefulFeatureFunction::StatefulFeatureFunction(const std::string& description, const std::string &line)
: FeatureFunction(description, line)
{
  m_statefulFFs.push_back(this);
}

StatefulFeatureFunction::StatefulFeatureFunction(const std::string& description, size_t numScoreComponents, const std::string &line)
: FeatureFunction(description,numScoreComponents, line)
{
  m_statefulFFs.push_back(this);
}

bool StatefulFeatureFunction::IsStateless() const
{
  return false;
}

}

