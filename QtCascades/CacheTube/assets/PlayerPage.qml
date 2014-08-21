import bb.cascades 1.0
import bb.system 1.0
import bb.multimedia 1.0
import CustomTimer 1.0

Page {
    id:                  playerPage
    objectName:          "playerPage"
    actionBarVisibility: controlsVisible ? ChromeVisibility.Default : ChromeVisibility.Hidden 

    property bool appInForeground: true
    property bool controlsVisible: true

    function playVideo(video_uri, video_title, auto_repeat) {
        playerPageTitleBar.title             = video_title;
        videoPlayer.sourceUrl                = video_uri;
        videoNowPlayingConnection.videoTitle = video_title;

        if (auto_repeat) {
            videoPlayer.repeatMode = RepeatMode.Track;
        } else {
            videoPlayer.repeatMode = RepeatMode.None;
        }

        if (videoNowPlayingConnection.acquire() !== MediaError.None) {
            videoPlaybackErrorToast.show();
        }
    }

    function disconnectSignals() {
        Application.fullscreen.disconnect(onAppInForeground);
        Application.invisible.disconnect(onAppInBackground);
        Application.thumbnail.disconnect(onAppInBackground);
    }

    function onAppInForeground() {
        appInForeground = true;
        
        if (videoNowPlayingConnection.acquired) {
            videoNowPlayingConnection.setOverlayStyle(OverlayStyle.Plain);
        }
    }

    function onAppInBackground() {
        appInForeground = false;
        
        if (videoNowPlayingConnection.acquired) {
            videoNowPlayingConnection.setOverlayStyle(OverlayStyle.Fancy);
        }
    }

    onCreationCompleted: {
        Application.fullscreen.connect(onAppInForeground);
        Application.invisible.connect(onAppInBackground);
        Application.thumbnail.connect(onAppInBackground);
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
                    title       = qsTr("Play");
                    imageSource = "images/play.png";
                } else {
                    title       = qsTr("Pause");
                    imageSource = "images/pause.png";
                }
            }
            
            onTriggered: {
                if (playAction) {
                    if (videoNowPlayingConnection.acquired) {
                        if (videoPlayer.play() !== MediaError.None) {
                            videoPlaybackErrorToast.show();
                        }
                    } else {
                        if (videoNowPlayingConnection.acquire() !== MediaError.None) {
                            videoPlaybackErrorToast.show();
                        }
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
                            if (videoNowPlayingConnection.acquired && videoPlayer.pause() !== MediaError.None) {
                                videoPlaybackErrorToast.show();
                            }
                        } else {
                            if (videoNowPlayingConnection.acquired && videoPlayer.play() !== MediaError.None) {
                                videoPlaybackErrorToast.show();
                            }
                        }
                    }
                },
                MediaKeyWatcher {
                    key: MediaKey.Play

                    onShortPress: {
                        if (videoNowPlayingConnection.acquired && videoPlayer.play() !== MediaError.None) {
                            videoPlaybackErrorToast.show();
                        }
                    }
                },
                MediaKeyWatcher {
                    key: MediaKey.Pause

                    onShortPress: {
                        if (videoNowPlayingConnection.acquired && videoPlayer.pause() !== MediaError.None) {
                            videoPlaybackErrorToast.show();
                        }
                    }
                },
                NowPlayingConnection {
                    id:              videoNowPlayingConnection
                    connectionName:  "CacheTube"
                    iconUrl:         "images/cachetube.png"
                    previousEnabled: false
                    nextEnabled:     false
                    duration:        videoPlayer.duration
                    position:        videoPlayer.position
                    mediaState:      videoPlayer.mediaState

                    property string videoTitle: ""
                    
                    onAcquired: {
                        var metadata = {"title": videoTitle, "artist": "CacheTube"};
                        
                        setMetaData(metadata);

                        if (playerPage.appInForeground) {
                            setOverlayStyle(OverlayStyle.Plain);
                        } else {
                            setOverlayStyle(OverlayStyle.Fancy);
                        }

                        if (videoPlayer.play() !== MediaError.None) {
                            videoPlaybackErrorToast.show();
                        }
                    }

                    onPlay: {
                        if (videoPlayer.play() !== MediaError.None) {
                            videoPlaybackErrorToast.show();
                        }
                    }
                    
                    onPause: {
                        if (videoPlayer.pause() !== MediaError.None) {
                            videoPlaybackErrorToast.show();
                        }
                    }
                    
                    onRevoked: {
                        if (videoPlayer.pause() !== MediaError.None) {
                            videoPlaybackErrorToast.show();
                        }
                    }
                }
            ]
        }

        Container {
            id:                  toolbarContainer
            horizontalAlignment: HorizontalAlignment.Fill
            verticalAlignment:   VerticalAlignment.Bottom
            background:          Color.Black
            opacity:             0.5
            visible:             playerPage.controlsVisible

            layout: StackLayout {
                orientation: LayoutOrientation.LeftToRight
            }

            function msToTime(msec) {
                var time = msec;
                
                var ms = time % 1000;
                time   = (time - ms) / 1000;
                
                var ss = time % 60;
                time   = (time - ss) / 60;
                
                var mm = time % 60;
                var hh = (time - mm) / 60;
                
                return (hh < 10 ? '0' : '') + hh + ":" + (mm < 10 ? '0' : '') + mm + ":" + (ss < 10 ? '0' : '') + ss; 
            }

            Label {
                text:               toolbarContainer.msToTime(videoPlayer.position)
                textFormat:         TextFormat.Plain
                textStyle.color:    Color.White
                textStyle.fontSize: FontSize.Small 
                
                layoutProperties: StackLayoutProperties {
                    spaceQuota: -1
                } 
            }

            Slider {
                id:        videoSlider
                fromValue: 0.0
                toValue:   videoPlayer.duration
                enabled:   videoPlayer.seekable
                
                property bool playbackWasActive: false
                
                layoutProperties: StackLayoutProperties {
                    spaceQuota: 1
                }
                
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
                            if (videoNowPlayingConnection.acquired) {
                                if (videoPlayer.play() !== MediaError.None) {
                                    videoPlaybackErrorToast.show();
                                }
                            } else {
                                if (videoNowPlayingConnection.acquire() !== MediaError.None) {
                                    videoPlaybackErrorToast.show();
                                }
                            }
                        }
                    }
                }
            }

            Label {
                text:               toolbarContainer.msToTime(videoPlayer.duration)
                textFormat:         TextFormat.Plain
                textStyle.color:    Color.White
                textStyle.fontSize: FontSize.Small 
                
                layoutProperties: StackLayoutProperties {
                    spaceQuota: -1
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
