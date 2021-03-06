#ifndef BDN_TEST_TestScrollViewCore_H_
#define BDN_TEST_TestScrollViewCore_H_

#include <bdn/test/TestViewCore.h>
#include <bdn/ScrollView.h>
#include <bdn/IScrollViewCore.h>

#include <bdn/test/ScrollViewLayoutTesterBase.h>

namespace bdn
{
    namespace test
    {

        /** Helper for tests that verify IScrollViewCore implementations.*/
        class TestScrollViewCore : public bdn::test::ScrollViewLayoutTesterBase<TestViewCore<Window>>
        {
          protected:
            P<View> createView() override
            {
                P<ScrollView> scrollView = newObj<ScrollView>();

                return scrollView;
            }

            void initCore() override
            {
                TestViewCore::initCore();

                _scrollViewCore = cast<IScrollViewCore>(_core);
            }

            void setView(View *view) override
            {
                TestViewCore::setView(view);

                _scrollView = cast<ScrollView>(view);
            }

            void runInitTests() override { TestViewCore::runInitTests(); }

            void runPostInitTests() override
            {
                TestViewCore::runPostInitTests();

                SECTION("preferred size and layout") { this->doPreferredSizeAndLayoutTests(); }

                SECTION("scrollClientRectToVisible and visibleClientRect") { this->testScrollClientRectToVisible(); }
            }

            P<ScrollView> getScrollView() override { return _scrollView; }

            Size callCalcPreferredSize(const Size &availableSpace = Size::none()) override
            {
                return _scrollViewCore->calcPreferredSize(availableSpace);
            }

            Size prepareCalcLayout(const Size &viewPortSize) override
            {
                // we must force the viewport to have the requested size.
                _viewPortSizeRequestedInPrepare = initiateScrollViewResizeToHaveViewPortSize(viewPortSize);

                // the final viewport size should be roughly the same as the
                // requested one
                REQUIRE(_viewPortSizeRequestedInPrepare >= viewPortSize - Size(5, 5));
                REQUIRE(_viewPortSizeRequestedInPrepare <= viewPortSize + Size(5, 5));

                return _viewPortSizeRequestedInPrepare;
            }

            void calcLayoutAfterPreparation() override
            {
                // verify that the scroll view has a plausible size.
                Size scrollViewSize = _scrollView->size();

                Rect expectedBounds(_scrollView->position(), _viewPortSizeRequestedInPrepare);
                expectedBounds = _scrollView->adjustBounds(expectedBounds, RoundType::nearest, RoundType::nearest);

                Size expectedSize = expectedBounds.getSize();

                REQUIRE(scrollViewSize >= expectedSize);

                // we do not know how big the viewport border is, but we assume
                // that it is less than 50 DIPs. If the following fails then it
                // might be the case that the border is actually bigger - in
                // that case this test must be adapted.
                REQUIRE(scrollViewSize <= expectedSize + Size(50, 50));

                // request layout explicitly again. Usually the resizing will
                // have caused one, but we want to make sure.
                return _scrollView->needLayout(View::InvalidateReason::customDataChanged);
            }

            /** Causes the scroll view to have the specified viewport size when
               no scroll bars are shown.

                It is ok if the resizing happens asynchronously after the
               function has already returned.

                Must return the viewport size that the view will actually end up
               having (adjusted or pixel aligned for the current display).
            */
            virtual Size initiateScrollViewResizeToHaveViewPortSize(const Size &viewPortSize) = 0;

            virtual void testScrollClientRectToVisible()
            {
                P<ScrollView> scrollView = getScrollView();
                P<Button> button = newObj<Button>();

                // make the button bigger than the scroll view so that
                // it will scroll
                button->setPreferredSizeMinimum(Size(1000, 1000));
                button->setPreferredSizeMaximum(Size(1000, 1000));

                scrollView->setHorizontalScrollingEnabled(true);
                scrollView->setContentView(button);

                initiateScrollViewResizeToHaveViewPortSize(Size(300, 300));

                P<TestScrollViewCore> self = this;

                CONTINUE_SECTION_WHEN_IDLE(self, button, scrollView)
                {
                    Size scrollViewSize = scrollView->size();
                    Size viewPortSize = scrollView->visibleClientRect().getSize();
                    Size clientSize = button->size();

                    // verify that the scroll view initialization was
                    // successful. If this fails then it can be that either the
                    // resizing of the scrollview was incorrect, or that the
                    // visibleClientRect property of the View was not correctly
                    // updated by the ScrollViewCore implementation.
                    REQUIRE(viewPortSize > Size(200, 200));
                    REQUIRE(viewPortSize < Size(400, 400));

                    REQUIRE(clientSize > Size(900, 900));
                    REQUIRE(clientSize < Size(1100, 1100));

                    SECTION("start to end")
                    {
                        SECTION("zero target area size")
                        {
                            subTestScrollClientRectToVisible(self, scrollView, 0, Size(), 0, clientSize, 0, Size(), 0,
                                                             clientSize - viewPortSize);
                        }

                        SECTION("nonzero target area size")
                        {
                            subTestScrollClientRectToVisible(self, scrollView, 0, Size(), -5, clientSize, 5, Size(), 0,
                                                             clientSize - viewPortSize);
                        }
                    }

                    SECTION("end to start")
                    {
                        SECTION("zero target area size")
                        {
                            subTestScrollClientRectToVisible(self, scrollView, 0, clientSize - viewPortSize, 0, Size(),
                                                             0, Size(), 0, Size());
                        }

                        SECTION("nonzero target area size")
                        {
                            subTestScrollClientRectToVisible(self, scrollView, 0, clientSize - viewPortSize, 0, Size(),
                                                             5, Size(), 0, Size());
                        }
                    }

                    SECTION("start to almost end")
                    {
                        SECTION("zero target area size")
                        {
                            subTestScrollClientRectToVisible(self, scrollView, 0, Size(), -5, clientSize, 0, Size(), -5,
                                                             clientSize - viewPortSize);
                        }

                        SECTION("nonzero target area size")
                        {
                            subTestScrollClientRectToVisible(self, scrollView, 0, Size(), -10, clientSize, 5, Size(),
                                                             -5, clientSize - viewPortSize);
                        }
                    }

                    SECTION("end to almost start")
                    {
                        SECTION("zero target area size")
                        {
                            subTestScrollClientRectToVisible(self, scrollView, 0, clientSize - viewPortSize, 5, Size(),
                                                             0, Size(), 5, Size());
                        }

                        SECTION("nonzero target area size")
                        {
                            subTestScrollClientRectToVisible(self, scrollView, 0, clientSize - viewPortSize, 5, Size(),
                                                             5, Size(), 5, Size());
                        }
                    }

                    SECTION("area already visible")
                    {
                        SECTION("start of viewport")
                        {
                            SECTION("zero target area size")
                            {
                                subTestScrollClientRectToVisible(self, scrollView, 10, Size(), 10, Size(), 0, Size(),
                                                                 10, Size());
                            }

                            SECTION("nonzero target area size")
                            {
                                subTestScrollClientRectToVisible(self, scrollView, 10, Size(), 10, Size(), 5, Size(),
                                                                 10, Size());
                            }
                        }

                        SECTION("end of viewport")
                        {
                            SECTION("zero target area size")
                            {
                                subTestScrollClientRectToVisible(self, scrollView, 10, Size(), 10, viewPortSize, 0,
                                                                 Size(), 10, Size());
                            }

                            SECTION("nonzero target area size")
                            {
                                subTestScrollClientRectToVisible(self, scrollView, 10, Size(), 5, viewPortSize, 5,
                                                                 Size(), 10, Size());
                            }
                        }
                    }

                    SECTION("part of target area barely not visible")
                    {
                        SECTION("start of viewport")
                        {
                            SECTION("zero target area size")
                            {
                                subTestScrollClientRectToVisible(self, scrollView, 10, Size(), 10 - 1, Size(), 0,
                                                                 Size(), 10 - 1, Size());
                            }

                            SECTION("nonzero target area size")
                            {
                                subTestScrollClientRectToVisible(self, scrollView, 10, Size(), 10 - 1, Size(), 5,
                                                                 Size(), 10 - 1, Size());
                            }
                        }

                        SECTION("end of viewport")
                        {
                            SECTION("zero target area size")
                            {
                                subTestScrollClientRectToVisible(self, scrollView, 10, Size(), 10 + 1, viewPortSize, 0,
                                                                 Size(), 10 + 1, Size());
                            }

                            SECTION("nonzero target area size")
                            {
                                subTestScrollClientRectToVisible(self, scrollView, 10, Size(), 10 + 1 - 5, viewPortSize,
                                                                 5, Size(), 10 + 1, Size());
                            }
                        }
                    }

                    SECTION("target position outside client area")
                    {
                        SECTION("negative position")
                        {
                            SECTION("target area ends before 0")
                            {
                                SECTION("zero target area size")
                                {
                                    subTestScrollClientRectToVisible(self, scrollView, 10, Size(), -100, Size(), 0,
                                                                     Size(), 0, Size());
                                }

                                SECTION("nonzero target area size")
                                {
                                    subTestScrollClientRectToVisible(self, scrollView, 10, Size(), -100, Size(), 5,
                                                                     Size(), 0, Size());
                                }
                            }

                            SECTION("negative infinity position")
                            {
                                SECTION("zero target area size")
                                {
                                    subTestScrollClientRectToVisible(self, scrollView, 10, Size(),
                                                                     -std::numeric_limits<double>::infinity(), Size(),
                                                                     0, Size(), 0, Size());
                                }

                                SECTION("nonzero target area size")
                                {
                                    // the size of the target area does not
                                    // matter if the position is negative
                                    // infinity.

                                    subTestScrollClientRectToVisible(self, scrollView, 10, Size(),
                                                                     -std::numeric_limits<double>::infinity(), Size(),
                                                                     300, Size(), 0, Size());
                                }
                            }

                            SECTION("target area crosses 0")
                            {
                                subTestScrollClientRectToVisible(self, scrollView, 10, Size(), -100, Size(), 150,
                                                                 Size(), 0, Size());
                            }

                            SECTION("target area crosses and exceeds viewport")
                            {
                                subTestScrollClientRectToVisible(self, scrollView, 10, Size(), -100, Size(), 150,
                                                                 viewPortSize,

                                                                 // the target rect is bigger than the
                                                                 // viewport, so we cannot make all of it
                                                                 // visible. The scroll view should scroll
                                                                 // the minimal amount possible to make as
                                                                 // much of the target rect visible as it
                                                                 // can. Since the current visible rect is
                                                                 // already fully in the target area that
                                                                 // means that the scroll view should not
                                                                 // scroll at all
                                                                 10, Size());
                            }
                        }

                        SECTION("target area bigger than viewport and already "
                                "visible")
                        {
                            subTestScrollClientRectToVisible(self, scrollView, 10, Size(), 5, Size(), 10, viewPortSize,

                                                             // should not move, since all the area of the
                                                             // target rect that can fit into the viewport is
                                                             // already visible
                                                             10, Size());
                        }

                        SECTION("target area bigger than viewport and begins "
                                "slightly after current scroll position")
                        {
                            subTestScrollClientRectToVisible(self, scrollView, 10, Size(), 11, Size(), 2, viewPortSize,

                                                             // should move one pixel to right, since the
                                                             // left edge of the target area is 1 pixel to
                                                             // right of current viewport
                                                             11, Size());
                        }

                        SECTION("target area bigger than viewport and ends "
                                "slightly before end of current visible rect")
                        {
                            subTestScrollClientRectToVisible(self, scrollView, 10, Size(), 5, Size(), 4, viewPortSize,

                                                             // should move one pixel to left since the right
                                                             // edge of the target area is 1 pixel to left of
                                                             // current viewport end
                                                             9, Size());
                        }

                        SECTION("exceeds end")
                        {
                            SECTION("target area starts after client area")
                            {
                                SECTION("zero target area size")
                                {
                                    subTestScrollClientRectToVisible(self, scrollView, 10, Size(), 1, clientSize, 0,
                                                                     Size(), 0, clientSize - viewPortSize);
                                }

                                SECTION("nonzero target area size")
                                {
                                    subTestScrollClientRectToVisible(self, scrollView, 10, Size(), 1, clientSize, 5,
                                                                     Size(), 0, clientSize - viewPortSize);
                                }
                            }

                            SECTION("target area crosses end")
                            {
                                subTestScrollClientRectToVisible(self, scrollView, 10, Size(), -10, clientSize, 20,
                                                                 Size(), 0, clientSize - viewPortSize);
                            }

                            SECTION("target area crosses end and exceeds viewport")
                            {
                                subTestScrollClientRectToVisible(self, scrollView, 10, Size(), -10, clientSize, 20,
                                                                 viewPortSize, 0, clientSize - viewPortSize);
                            }
                        }

                        SECTION("positive infinity")
                        {
                            SECTION("zero target area size")
                            {
                                subTestScrollClientRectToVisible(self, scrollView, 10, Size(),
                                                                 std::numeric_limits<double>::infinity(), Size(), 0,
                                                                 Size(), 0, clientSize - viewPortSize);
                            }

                            SECTION("nonzero target area size")
                            {
                                subTestScrollClientRectToVisible(self, scrollView, 10, Size(),
                                                                 std::numeric_limits<double>::infinity(), Size(), 5,
                                                                 Size(), 0, clientSize - viewPortSize);
                            }
                        }

                        SECTION("negative infinity")
                        {
                            SECTION("zero target area size")
                            {
                                subTestScrollClientRectToVisible(self, scrollView, 10, Size(),
                                                                 -std::numeric_limits<double>::infinity(), Size(), 0,
                                                                 Size(), 0, Size());
                            }

                            SECTION("nonzero target area size")
                            {
                                subTestScrollClientRectToVisible(self, scrollView, 10, Size(),
                                                                 -std::numeric_limits<double>::infinity(), Size(), 5,
                                                                 Size(), 0, Size());
                            }
                        }
                    }
                };
            }

            P<ScrollView> _scrollView;
            P<IScrollViewCore> _scrollViewCore;

          private:
            enum class TestDir_
            {
                horz,
                vert
            };

            static double &comp(Size &s, TestDir_ dir)
            {
                if (dir == TestDir_::horz)
                    return s.width;
                else
                    return s.height;
            }

            static double &comp(Point &p, TestDir_ dir)
            {
                if (dir == TestDir_::horz)
                    return p.x;
                else
                    return p.y;
            }

            static Point compToPoint(double c, TestDir_ dir)
            {
                if (dir == TestDir_::horz)
                    return Point(c, 0);
                else
                    return Point(0, c);
            }

            static Size compToSize(double s, TestDir_ dir)
            {
                if (dir == TestDir_::horz)
                    return Size(s, 0);
                else
                    return Size(0, s);
            }

            static void waitForCondition(int timeoutMillisLeft, std::function<bool(bool)> checkFunc,
                                         std::function<void()> continueFunc)
            {
                // instead of waiting for a fixed time on the clock we do a
                // fixed number of check steps, with a small delay in between.
                // That has the advantage that it will automatically wait longer
                // if the test process gets suspended or if the system load is
                // so high that the process does not get much cpu time. That is
                // what we want.

                const int stepDelayMillis = 100;

                bool lastTry = (timeoutMillisLeft <= stepDelayMillis);

                if (!checkFunc(lastTry)) {
                    timeoutMillisLeft -= stepDelayMillis;

                    std::function<void()> f = [timeoutMillisLeft, checkFunc, continueFunc]() {
                        waitForCondition(timeoutMillisLeft, checkFunc, continueFunc);
                    };

                    BDN_CONTINUE_SECTION_AFTER_ABSOLUTE_SECONDS_WITH(((double)stepDelayMillis) / 1000, f);
                } else {
                    continueFunc();
                }
            }

            static void subTestScrollClientRectToVisible_Dir(TestDir_ dir, P<IBase> keepAliveDuringTest,
                                                             P<ScrollView> scrollView, double initialPos,
                                                             Size initialPosAdd, double targetPos, Size targetPosAdd,
                                                             double targetSize, Size targetSizeAdd, double expectedPos,
                                                             Size expectedPosAdd)
            {
                Size scrolledAreaSize = scrollView->getContentView()->size();

                initialPos += comp(initialPosAdd, dir);
                targetPos += comp(targetPosAdd, dir);
                targetSize += comp(targetSizeAdd, dir);
                expectedPos += comp(expectedPosAdd, dir);

                Rect visibleRectBefore = scrollView->visibleClientRect();

                scrollView->scrollClientRectToVisible(
                    Rect(compToPoint(initialPos, dir), scrollView->visibleClientRect().getSize()));

                // it may take a while until the scroll operation is done (for
                // example, if it is animated). So we wait and check a few times
                // until the expected condition is present.
                waitForCondition(
                    10 * 1000,
                    [keepAliveDuringTest, scrollView, initialPos, dir, visibleRectBefore](bool lastTry) {
                        Rect visibleRect = scrollView->visibleClientRect();

                        Rect expectedInitialRect(compToPoint(initialPos, dir), visibleRectBefore.getSize());
                        Rect adjustedExpectedInitialRect_down = scrollView->getContentView()->adjustBounds(
                            expectedInitialRect, RoundType::down, RoundType::nearest);
                        Rect adjustedExpectedInitialRect_up = scrollView->getContentView()->adjustBounds(
                            expectedInitialRect, RoundType::up, RoundType::nearest);

                        if (lastTry) {
                            REQUIRE_ALMOST_EQUAL(
                                visibleRect.x, expectedInitialRect.x,
                                std::fabs(adjustedExpectedInitialRect_up.x - adjustedExpectedInitialRect_down.x));
                            REQUIRE_ALMOST_EQUAL(
                                visibleRect.y, expectedInitialRect.y,
                                std::fabs(adjustedExpectedInitialRect_up.y - adjustedExpectedInitialRect_down.y));

                            return true;
                        } else {
                            return (
                                std::fabs(visibleRect.x - expectedInitialRect.x) <=
                                    std::fabs(adjustedExpectedInitialRect_up.x - adjustedExpectedInitialRect_down.x) &&
                                std::fabs(visibleRect.y - expectedInitialRect.y) <=
                                    std::fabs(adjustedExpectedInitialRect_up.y - adjustedExpectedInitialRect_down.y));
                        }
                    },
                    [keepAliveDuringTest, scrollView, targetPos, targetSize, expectedPos, dir, visibleRectBefore,
                     initialPos, initialPosAdd, targetPosAdd, targetSizeAdd, expectedPosAdd]() {
                        scrollView->scrollClientRectToVisible(
                            Rect(compToPoint(targetPos, dir), compToSize(targetSize, dir)));

                        waitForCondition(
                            10 * 1000,
                            [keepAliveDuringTest, scrollView, targetPos, targetSize, expectedPos, dir,
                             visibleRectBefore, initialPos, initialPosAdd, targetPosAdd, targetSizeAdd,
                             expectedPosAdd](bool lastTry) {
                                Rect visibleRect = scrollView->visibleClientRect();

                                Rect expectedRect(compToPoint(expectedPos, dir), visibleRectBefore.getSize());
                                Rect adjustedExpectedRect_down = scrollView->getContentView()->adjustBounds(
                                    expectedRect, RoundType::down, RoundType::nearest);
                                Rect adjustedExpectedRect_up = scrollView->getContentView()->adjustBounds(
                                    expectedRect, RoundType::up, RoundType::nearest);

                                // copy captured variables to locals for better
                                // debugging in lldb
                                TestDir_ dirL = dir;
                                double initialPosL = initialPos;
                                Size initialPosAddL = initialPosAdd;
                                double targetPosL = targetPos;
                                Size targetPosAddL = targetPosAdd;
                                double targetSizeL = targetSize;
                                Size targetSizeAddL = targetSizeAdd;
                                double expectedPosL = expectedPos;
                                Size expectedPosAddL = expectedPosAdd;

                                if (lastTry) {
                                    // allow the visible rect to be adjusted
                                    // just like a view bounds rect would be.
                                    REQUIRE_ALMOST_EQUAL(
                                        visibleRect.x, expectedRect.x,
                                        std::fabs(adjustedExpectedRect_up.x - adjustedExpectedRect_down.x));
                                    REQUIRE_ALMOST_EQUAL(
                                        visibleRect.y, expectedRect.y,
                                        std::fabs(adjustedExpectedRect_up.y - adjustedExpectedRect_down.y));

                                    return true;
                                } else {
                                    return (std::fabs(visibleRect.x - expectedRect.x) <=
                                                std::fabs(adjustedExpectedRect_up.x - adjustedExpectedRect_down.x) &&
                                            std::fabs(visibleRect.y - expectedRect.y) <=
                                                std::fabs(adjustedExpectedRect_up.y - adjustedExpectedRect_down.y));
                                }
                            },
                            [keepAliveDuringTest, scrollView, visibleRectBefore]() {
                                Rect visibleRect = scrollView->visibleClientRect();

                                // Size should not have changed
                                REQUIRE(visibleRect.getSize() == visibleRectBefore.getSize());
                            });
                    });
            }

            static void subTestScrollClientRectToVisible(P<IBase> keepAliveDuringTest, P<ScrollView> scrollView,
                                                         double initialPos, Size initialPosAdd, double targetPos,
                                                         Size targetPosAdd, double targetSize, Size targetSizeAdd,
                                                         double expectedPos, Size expectedPosAdd)
            {
                SECTION("vertical")
                {
                    subTestScrollClientRectToVisible_Dir(TestDir_::vert, keepAliveDuringTest, scrollView, initialPos,
                                                         initialPosAdd, targetPos, targetPosAdd, targetSize,
                                                         targetSizeAdd, expectedPos, expectedPosAdd);
                }

                SECTION("horizontal")
                {
                    subTestScrollClientRectToVisible_Dir(TestDir_::horz, keepAliveDuringTest, scrollView, initialPos,
                                                         initialPosAdd, targetPos, targetPosAdd, targetSize,
                                                         targetSizeAdd, expectedPos, expectedPosAdd);
                }
            }

            Size _viewPortSizeRequestedInPrepare;
        };
    }
}

#endif
