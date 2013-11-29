import bb.cascades 1.0
import bb.system 1.0
import PlayerLauncher 1.0
import YTVideoManagement 1.0

TabbedPane {
    id: tabbedPane
    
    Tab {
        id:          youTubeTab
        title:       qsTr("YouTube")
        imageSource: "images/youtube.png"

        Page {
            actions: [
                ActionItem {
                    title:               qsTr("Back")
                    imageSource:         "images/back.png"
                    ActionBar.placement: ActionBarPlacement.OnBar

                    onTriggered: {
                        youTubeWebView.goBack();
                    }
                },
                ActionItem {
                    title:               qsTr("Home")
                    imageSource:         "images/home.png"
                    ActionBar.placement: ActionBarPlacement.OnBar

                    onTriggered: {
                        youTubeWebView.url = "http://m.youtube.com/";
                    }
                },
                ActionItem {
                    title:               qsTr("Cache")
                    imageSource:         "images/cache.png"
                    ActionBar.placement: ActionBarPlacement.OnBar

                    onTriggered: {
                        var video_id = YTVideoManager.getVideoId(youTubeWebView.url);
                        
                        if (video_id !== "") {
                            if (YTVideoManager.addTask(video_id)) {
                                cachingStartedToast.show();
                            } else {
                                cachingFailedToast.show();
                            }
                        }
                    }
                    
                    attachedObjects: [
                        SystemToast {
                            id:   cachingStartedToast
                            body: qsTr("Video caching started") 
                        },
                        SystemToast {
                            id:   cachingFailedToast
                            body: qsTr("Could not cache this video. May be it is already cached?")
                        }
                    ]
                },
                ActionItem {
                    title:               qsTr("Reload")
                    imageSource:         "images/reload.png"
                    ActionBar.placement: ActionBarPlacement.InOverflow

                    onTriggered: {
                        youTubeWebView.reload();
                    }
                }
            ]

            Container {
                background: Color.White
                
                layout: DockLayout {
                }

                ScrollView {
                    horizontalAlignment: HorizontalAlignment.Fill
                    verticalAlignment:   VerticalAlignment.Fill
                    
                    scrollViewProperties {
                        scrollMode:         ScrollMode.Both
                        pinchToZoomEnabled: true
                        minContentScale:    1.0
                        maxContentScale:    4.0
                    }

                    WebView {
                        id:  youTubeWebView
                        url: "http://m.youtube.com/"
                    }
                }
                
                ProgressIndicator {
                    horizontalAlignment: HorizontalAlignment.Fill
                    verticalAlignment:   VerticalAlignment.Bottom
                    visible:             youTubeWebView.loading
                    fromValue:           0
                    toValue:             100
                    value:               youTubeWebView.loadProgress
                }
            }
        }
    }
    
    Tab {
        title:       qsTr("Cached Video")
        imageSource: "images/cache.png"

        Page {
            Container {
                background: Color.White

                layout: DockLayout {
                }

                Container {
                    horizontalAlignment: HorizontalAlignment.Center
                    verticalAlignment:   VerticalAlignment.Center
                    background:          Color.Transparent
                    visible:             cacheListViewDataModel.itemsCount <= 0
                    
                    layout: StackLayout {
                    }
                    
                    ImageView {
                        horizontalAlignment: HorizontalAlignment.Center
                        imageSource:         "images/cache_empty.png"
                    }
                    
                    Label {
                        horizontalAlignment: HorizontalAlignment.Center
                        textStyle.color:     Color.Black
                        textStyle.fontSize:  FontSize.XLarge
                        text:                qsTr("Video cache is empty")
                    }
                }

                ListView {
                    horizontalAlignment: HorizontalAlignment.Fill
                    verticalAlignment:   VerticalAlignment.Fill
                    visible:             cacheListViewDataModel.itemsCount > 0

                    property variant ytVideoManager: YTVideoManager 

                    dataModel: YTArrayDataModel {
                        id:           cacheListViewDataModel
                        videoManager: YTVideoManager
                    }
                    
                    function navigateToWebPage(url) {
                        youTubeWebView.url = url;

                        tabbedPane.activeTab = youTubeTab;
                    } 
                    
                    listItemComponents: [
                        ListItemComponent {
                            type: ""
                            
                            Container {
                                id:           itemRoot
                                background:   Color.Transparent 
                                leftPadding:  12
                                rightPadding: 12

                                property int    itemState:    ListItemData.state
                                property int    itemSize:     ListItemData.size
                                property int    itemDone:     ListItemData.done
                                property string itemVideoId:  ListItemData.videoId
                                property string itemMimeType: ListItemData.mimeType
                                property string itemTitle:    ListItemData.title
                                property string itemErrorMsg: ListItemData.errorMsg

                                layout: StackLayout {
                                    orientation: LayoutOrientation.TopToBottom
                                }

                                contextActions: [
                                    ActionSet {
                                        title: qsTr("Manage Video")

                                        ActionItem {
                                            title:       qsTr("Delete")
                                            imageSource: "images/delete.png"
                                            
                                            onTriggered: {
                                                itemRoot.ListItem.view.ytVideoManager.delTask(itemRoot.itemVideoId);
                                            }
                                        }

                                        ActionItem {
                                            title:       qsTr("Pause/Resume")
                                            imageSource: "images/pause_resume.png"
                                            enabled:     itemRoot.itemState !== YTDownloadState.StateCompleted

                                            onTriggered: {
                                                if (itemRoot.itemState === YTDownloadState.StatePaused) {
                                                    itemRoot.ListItem.view.ytVideoManager.resumeTask(itemRoot.itemVideoId);
                                                } else {
                                                    itemRoot.ListItem.view.ytVideoManager.pauseTask(itemRoot.itemVideoId);
                                                }
                                            }
                                        }

                                        ActionItem {
                                            title:       qsTr("Open YouTube Page")
                                            imageSource: "images/youtube.png"

                                            onTriggered: {
                                                itemRoot.ListItem.view.navigateToWebPage(itemRoot.ListItem.view.ytVideoManager.getTaskWebURL(itemRoot.itemVideoId));
                                            }
                                        }

                                        ActionItem {
                                            title:       qsTr("Play Video")
                                            imageSource: "images/play_video.png"
                                            enabled:     itemRoot.itemState === YTDownloadState.StateCompleted

                                            onTriggered: {
                                                if (!playerLauncher.launchPlayer(itemRoot.ListItem.view.ytVideoManager.getTaskVideoURI(itemRoot.itemVideoId), itemRoot.itemMimeType, itemRoot.itemTitle)) {
                                                    videoPlayerFailedToast.show();                                                    
                                                }
                                            }
                                            
                                            attachedObjects: [
                                                PlayerLauncher {
                                                    id: playerLauncher
                                                },
                                                SystemToast {
                                                    id:   videoPlayerFailedToast
                                                    body: qsTr("Could not start video player")
                                                }
                                            ]
                                        }
                                    }
                                ]

                                Label {
                                    multiline:            true
                                    textFormat:           TextFormat.Plain
                                    textStyle.color:      Color.Black
                                    textStyle.fontWeight: FontWeight.Bold
                                    textStyle.fontSize:   FontSize.Medium
                                    text:                 itemRoot.itemTitle
                                }

                                ProgressIndicator {
                                    visible:   itemRoot.itemState === YTDownloadState.StateActive
                                    fromValue: 0
                                    toValue:   itemRoot.itemSize
                                    value:     itemRoot.itemDone
                                }

                                Label {
                                    visible:             itemRoot.itemState !== YTDownloadState.StateActive
                                    multiline:           true
                                    textStyle.color:     itemRoot.itemState === YTDownloadState.StateError ? Color.Red : Color.Grey
                                    textStyle.fontStyle: FontStyle.Italic
                                    textStyle.fontSize:  FontSize.Small
                                    text:                itemRoot.itemState === YTDownloadState.StateCompleted ? (itemRoot.itemSize / 1048576).toFixed(2) + "M" :
                                                        (itemRoot.itemState === YTDownloadState.StateError     ? itemRoot.itemErrorMsg                          :
                                                        (itemRoot.itemState === YTDownloadState.StateQueued    ? qsTr("QUEUED")                                 :
                                                        (itemRoot.itemState === YTDownloadState.StatePaused    ? qsTr("PAUSED")                                 : "")))
                                }

                                Divider {
                                }
                            }
                        }
                    ]
                }
            }
        }
    }
    
    Tab {
        title:       qsTr("Help")
        imageSource: "images/help.png" 
        
        Page {
            actions: [
                ActionItem {
                    title:               qsTr("Review App")
                    imageSource:         "images/review.png"
                    ActionBar.placement: ActionBarPlacement.OnBar

                    onTriggered: {
                        appWorldInvocation.trigger("bb.action.OPEN");
                    }

                    attachedObjects: [
                        Invocation {
                            id: appWorldInvocation

                            query: InvokeQuery {
                                mimeType: "application/x-bb-appworld"
                                uri:      "appworld://content/20385658"
                            }
                        }
                    ]
                }
            ]

            ScrollView {
                scrollViewProperties {
                    scrollMode:         ScrollMode.Both
                    pinchToZoomEnabled: true
                    minContentScale:    1.0
                    maxContentScale:    4.0
                }

                WebView {
                    url: qsTr("local:///assets/doc/help.html")
                }
            }
        }
    }
}
