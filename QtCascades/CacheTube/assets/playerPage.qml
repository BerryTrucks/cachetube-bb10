import bb.cascades 1.0
import bb.system 1.0
import bb.multimedia 1.0
import CustomTimer 1.0

Page {
    id:                  playerPage
    objectName:          "playerPage"
    actionBarVisibility: controlsVisible ? ChromeVisibility.Default : ChromeVisibility.Hidden 

    property bool controlsVisible: true

    function playVideo(video_uri, video_title) {
        playerPageTitleBar.title = video_title;
        videoPlayer.sourceUrl    = video_uri;

        if (videoPlayer.play() !== MediaError.None) {
            videoPlaybackErrorToast.show();
        }
    }

    titleBar: TitleBar {
        id:         playerPageTitleBar
        visibility: controlsVisible ? ChromeVisibility.Default : ChromeVisibility.Hidden
    }

    paneProperties: NavigationPaneProperties {
        backButton: ActionItem {
            onTriggered: {
                playerNavigationPane.pop();
            }
        }
    }

    actions: [
        ActionItem {
            id:                  playPauseActionItem
            title:               qsTr("Play")
            imageSource:         "images/play.png"
            ActionBar.placement: ActionBarPlacement.OnBar

            property bool playAction: true
            
            onPlayActionChanged: {
                if (playAction) {
                    playPauseActionItem.title       = qsTr("Play");
                    playPauseActionItem.imageSource = "images/play.png";
                } else {
                    playPauseActionItem.title       = qsTr("Pause");
                    playPauseActionItem.imageSource = "images/pause.png";
                }
            }
            
            onTriggered: {
                if (playAction) {
                    if (videoPlayer.play() !== MediaError.None) {
                        videoPlaybackErrorToast.show();
                    }
                } else {
                    if (videoPlayer.pause() !== MediaError.None) {
                        videoPlaybackErrorToast.show();
                    }
                }
            }
        }
    ]

    Container {
        id:         videoContainer
        background: Color.Black

        onTouch: {
            if (event.touchType === TouchType.Down) {
                playerPage.controlsVisible = true;

                if (videoPlayer.playbackActive) {
                    idleTimer.start();
                }
            }
        }

        layout: DockLayout {
        }

        ForeignWindowControl {
            id:                  videoForeignWindowControl
            windowId:            "videoForeignWindowControl"
            horizontalAlignment: HorizontalAlignment.Center
            verticalAlignment:   VerticalAlignment.Center
            preferredWidth:      0
            preferredHeight:     0
            visible:             false
            updatedProperties:   WindowProperty.Size | WindowProperty.Position | WindowProperty.Visible

            onBoundToWindowChanged: {
                if (boundToWindow && preferredWidth !== 0 && preferredHeight !== 0) {
                    visible = true;
                } else {
                    visible = false;
                }
            }

            onPreferredWidthChanged: {
                if (boundToWindow && preferredWidth !== 0 && preferredHeight !== 0) {
                    visible = true;
                } else {
                    visible = false;
                }
            }

            onPreferredHeightChanged: {
                if (boundToWindow && preferredWidth !== 0 && preferredHeight !== 0) {
                    visible = true;
                } else {
                    visible = false;
                }
            }

            attachedObjects: [
                MediaPlayer {
                    id:          videoPlayer
                    videoOutput: VideoOutput.PrimaryDisplay
                    windowId:    videoForeignWindowControl.windowId
                    repeatMode:  RepeatMode.None

                    property bool playbackActive: false
                    property bool videoSeeking:   false
                    property int  videoWidth:     0
                    property int  videoHeight:    0

                    onMediaStateChanged: {
                        if (mediaState === MediaState.Started) {
                            playbackActive = true;
                            
                            playPauseActionItem.playAction = false;

                            idleTimer.start();
                        } else {
                            playbackActive = false;

                            if (!videoSeeking) {
                                playPauseActionItem.playAction = true;
                            }

                            playerPage.controlsVisible = true;

                            idleTimer.stop();
                        }
                    }

                    onVideoDimensionsChanged: {
                        videoWidth  = videoDimensions.width;
                        videoHeight = videoDimensions.height;

                        var width  = videoContainerLayoutUpdateHandler.layoutFrame.width;
                        var height = videoContainerLayoutUpdateHandler.layoutFrame.height;

                        if (videoWidth !== 0 && videoHeight !== 0) {
                            var ratio = Math.min(width / videoWidth, height / videoHeight);

                            videoForeignWindowControl.preferredWidth  = videoWidth  * ratio;
                            videoForeignWindowControl.preferredHeight = videoHeight * ratio;
                        }
                    }
                    
                    onPositionChanged: {
                        if (!videoSeeking) {
                            videoSlider.value = position;
                        }
                    }
                    
                    onError: {
                        videoPlaybackErrorToast.show();
                    }
                },
                MediaKeyWatcher {
                    key: MediaKey.PlayPause
                    
                    onShortPress: {
                        if (videoPlayer.playbackActive) {
                            if (videoPlayer.pause() !== MediaError.None) {
                                videoPlaybackErrorToast.show();
                            }
                        } else {
                            if (videoPlayer.play() !== MediaError.None) {
                                videoPlaybackErrorToast.show();
                            }
                        }
                    }
                },
                MediaKeyWatcher {
                    key: MediaKey.Play

                    onShortPress: {
                        if (!videoPlayer.playbackActive) {
                            if (videoPlayer.play() !== MediaError.None) {
                                videoPlaybackErrorToast.show();
                            }
                        }
                    }
                },
                MediaKeyWatcher {
                    key: MediaKey.Pause

                    onShortPress: {
                        if (videoPlayer.playbackActive) {
                            if (videoPlayer.pause() !== MediaError.None) {
                                videoPlaybackErrorToast.show();
                            }
                        }
                    }
                }
            ]
        }

        Slider {
            id:                  videoSlider
            horizontalAlignment: HorizontalAlignment.Fill
            verticalAlignment:   VerticalAlignment.Bottom
            fromValue:           0.0
            toValue:             videoPlayer.duration
            enabled:             videoPlayer.seekable
            visible:             playerPage.controlsVisible

            property bool playbackWasActive: false

            onTouch: {
                if (event.touchType === TouchType.Down) {
                    playbackWasActive = videoPlayer.playbackActive; 
                    
                    videoPlayer.videoSeeking = true;

                    if (videoPlayer.pause() !== MediaError.None) {
                        videoPlaybackErrorToast.show();
                    }
                } else if (event.touchType === TouchType.Up || event.touchType === TouchType.Cancel) {
                    videoPlayer.seekTime(immediateValue);

                    videoPlayer.videoSeeking = false;

                    if (playbackWasActive) {
                        if (videoPlayer.play() !== MediaError.None) {
                            videoPlaybackErrorToast.show();
                        }
                    }
                }
            }
        }
        
        attachedObjects: [
            LayoutUpdateHandler {
                id: videoContainerLayoutUpdateHandler
                
                onLayoutFrameChanged: {
                    var width  = layoutFrame.width;
                    var height = layoutFrame.height;

                    if (videoPlayer.videoWidth !== 0 && videoPlayer.videoHeight !== 0) {
                        var ratio = Math.min(width / videoPlayer.videoWidth, height / videoPlayer.videoHeight);

                        videoForeignWindowControl.preferredWidth  = videoPlayer.videoWidth  * ratio;
                        videoForeignWindowControl.preferredHeight = videoPlayer.videoHeight * ratio;
                    }
                }
            }
        ]
    }
    
    attachedObjects: [
        SystemToast {
            id:   videoPlaybackErrorToast
            body: qsTr("An error occured during video playback")
        },
        CustomTimer {
            id:         idleTimer
            singleShot: true
            interval:   3000
            
            onTimeout: {
                playerPage.controlsVisible = false;
            }
        }
    ]
}
