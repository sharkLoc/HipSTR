#ifndef REPEAT_BLOCK_H_
#define REPEAT_BLOCK_H_

#include <algorithm>
#include <set>
#include <vector>

#include "../error.h"
#include "../stutter_model.h"
#include "HapBlock.h"
#include "RepeatStutterInfo.h"
#include "StutterAligner.h"

class RepeatBlock : public HapBlock {
 private:
    RepeatStutterInfo* repeat_info_;
    std::vector<StutterAligner*> stutter_aligners_;
    bool reversed_;

    // Private unimplemented copy constructor and assignment operator to prevent operations
    RepeatBlock(const RepeatBlock& other);
    RepeatBlock& operator=(const RepeatBlock& other);

 public:
 RepeatBlock(int32_t start, int32_t end, const std::string& ref_seq, bool vcf_based, int period, const StutterModel* stutter_model,
	     const bool reversed=false): HapBlock(start, end, ref_seq, vcf_based){
      repeat_info_ = new RepeatStutterInfo(period, ref_seq, stutter_model);
      reversed_    = reversed;
      stutter_aligners_.push_back(new StutterAligner(ref_seq, period, !reversed_, repeat_info_));
    }
    
    ~RepeatBlock(){
      delete repeat_info_;
      for (unsigned int i = 0; i < stutter_aligners_.size(); i++)
	delete stutter_aligners_[i];
      stutter_aligners_.clear();
    }

    void add_alternate(const std::string& alt, bool removable);

    StutterAligner* get_stutter_aligner(int seq_index)  { return stutter_aligners_[seq_index]; }
    RepeatStutterInfo* get_repeat_info()                { return repeat_info_; }

    HapBlock* reverse(){
      std::string rev_ref_seq = seqs_[0];
      std::reverse(rev_ref_seq.begin(), rev_ref_seq.end());
      RepeatBlock* rev_block = new RepeatBlock(start_, end_, rev_ref_seq, vcf_based_, repeat_info_->get_period(),
					       repeat_info_->get_stutter_model(), !reversed_);
      for (unsigned int i = 1; i < seqs_.size(); ++i){
	std::string alt = seqs_[i];
	std::reverse(alt.begin(), alt.end());
	rev_block->add_alternate(alt, removable_[i]);
      }
      return rev_block;
    }

    RepeatBlock* remove_alleles(const std::vector<int>& allele_indices){
      std::set<int> bad_alleles(allele_indices.begin(), allele_indices.end());
      assert(bad_alleles.find(0) == bad_alleles.end());
      RepeatBlock* new_block = new RepeatBlock(start_, end_, seqs_[0], vcf_based_, repeat_info_->get_period(),
					       repeat_info_->get_stutter_model(), reversed_);
      for (unsigned int i = 1; i < seqs_.size(); ++i){
	if (bad_alleles.find(i) == bad_alleles.end())
	  new_block->add_alternate(seqs_[i], removable_[i]);
	else
	  assert(removable_[i]);
      }
      return new_block;
    }
};

#endif
