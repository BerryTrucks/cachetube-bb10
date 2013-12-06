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
        videoPlayer.sourceUrl = video_uri;
        videoLabel.text       = video_title;
        
        if (videoPlayer.play() !== MediaError.None) {
            videoPlaybackErrorToast.show();
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
            visible:             boundToWindow
            updatedProperties:   WindowProperty.Size | WindowProperty.Position | WindowProperty.Visible

            attachedObjects: [
                MediaPlayer {
                    id:          videoPlayer
                    videoOutput: VideoOutput.PrimaryDisplay
                    windowId:    videoForeignWindowControl.windowId
                    repeatMode:  RepeatMode.None

                    property bool playbackActive: false
                    property int  videoWidth:     0
                    property int  videoHeight:    0

                    onMediaStateChanged: {
                        if (mediaState === MediaState.Started) {
                            playbackActive = true;
                            
                            playPauseActionItem.playAction = false;

                            idleTimer.start();
                        } else {
                            playbackActive = false;

                            playerPage.controlsVisible     = true;
                            playPauseActionItem.playAction = true;
                            
                            idleTimer.stop();
                        }
                    }

                    onVideoDimensionsChanged: {
                        videoWidth  = videoDimensions.width;
                        videoHeight = videoDimensions.height;

                        var width  = videoContainerLayoutUpdateHandler.layoutFrame.width;
                        var height = videoContainerLayoutUpdateHandler.layoutFrame.height;

                        if (videoWidth !== 0 && videoHeight !== 0) {
                            if (width < height) {
                                videoForeignWindowControl.preferredWidth  = width;
                                videoForeignWindowControl.preferredHeight = (videoHeight * width) / videoWidth;
                            } else {
                                videoForeignWindowControl.preferredWidth  = (videoWidth * height) / videoHeight;
                                videoForeignWindowControl.preferredHeight = height;
                            }
                        }
                    }
                    
                    onPositionChanged: {
                        if (!videoSlider.seeking) {
                            videoSlider.value = position;
                        }
                    }
                    
                    onError: {
                        videoPlaybackErrorToast.show();
                    }
                }
            ]
        }

        Label {
            id:                  videoLabel
            horizontalAlignment: HorizontalAlignment.Left
            verticalAlignment:   VerticalAlignment.Top
            textStyle.color:     Color.White
            textStyle.fontSize:  FontSize.XLarge
            visible:             playerPage.controlsVisible
        }

        Slider {
            id:                  videoSlider
            horizontalAlignment: HorizontalAlignment.Fill
            verticalAlignment:   VerticalAlignment.Bottom
            fromValue:           0.0
            toValue:             videoPlayer.duration
            enabled:             videoPlayer.seekable
            visible:             playerPage.controlsVisible

            property bool playbackActive: false
            property bool seeking:        false

            onTouch: {
                if (event.touchType === TouchType.Down) {
                    playbackActive = videoPlayer.playbackActive; 
                    seeking        = true;

                    if (videoPlayer.pause() !== MediaError.None) {
                        videoPlaybackErrorToast.show();
                    }
                } else if (event.touchType === TouchType.Up || event.touchType === TouchType.Cancel) {
                    seeking = false;

                    if (playbackActive) {
                        if (videoPlayer.play() !== MediaError.None) {
                            videoPlaybackErrorToast.show();
                        }
                    }
                }
            }

            onImmediateValueChanged: {
                if (seeking) {
                    videoPlayer.seekTime(immediateValue);
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
                        if (width < height) {
                            videoForeignWindowControl.preferredWidth  = width;
                            videoForeignWindowControl.preferredHeight = (videoPlayer.videoHeight * width)  / videoPlayer.videoWidth;
                        } else {
                            videoForeignWindowControl.preferredWidth  = (videoPlayer.videoWidth  * height) / videoPlayer.videoHeight;
                            videoForeignWindowControl.preferredHeight = height;
                        }
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
