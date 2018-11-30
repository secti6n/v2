// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_VIEWS_PAGE_ACTION_ZOOM_VIEW_H_
#define CHROME_BROWSER_UI_VIEWS_PAGE_ACTION_ZOOM_VIEW_H_

#include "base/macros.h"
#include "chrome/browser/ui/views/location_bar/location_bar_view.h"
#include "chrome/browser/ui/views/page_action/page_action_icon_view.h"

// View for the zoom icon in the Omnibox.
class ZoomView : public PageActionIconView {
 public:
  // Clicking on the ZoomView shows a ZoomBubbleView, which requires the current
  // WebContents. Because the current WebContents changes as the user switches
  // tabs, a LocationBarView::Delegate is supplied to queried for the current
  // WebContents when needed.
  ZoomView(LocationBarView::Delegate* location_bar_delegate,
           PageActionIconView::Delegate* delegate);
  ~ZoomView() override;

  // Updates the image and its tooltip appropriately, hiding or showing the icon
  // as needed.
  void ZoomChangedForActiveTab(bool can_show_bubble);

 protected:
  // PageActionIconView:
  bool Update() override;
  void OnExecuting(PageActionIconView::ExecuteSource source) override;
  views::BubbleDialogDelegateView* GetBubble() const override;
  const gfx::VectorIcon& GetVectorIcon() const override;
  base::string16 GetTextForTooltipAndAccessibleName() const override;

 private:
  bool ShouldBeVisible(bool can_show_bubble) const;

  // The delegate used to get whether omnibox input is in progress.
  LocationBarView::Delegate* location_bar_delegate_;

  const gfx::VectorIcon* icon_ = nullptr;

  int current_zoom_percent_ = 100;

  DISALLOW_COPY_AND_ASSIGN(ZoomView);
};

#endif  // CHROME_BROWSER_UI_VIEWS_PAGE_ACTION_ZOOM_VIEW_H_