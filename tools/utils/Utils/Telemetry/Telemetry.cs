﻿//-----------------------------------------------------------------------
// <copyright file="Telemetry.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Packaging.Utils.Telemetry
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Threading;
    using Microsoft.Diagnostics.Telemetry;
    using Microsoft.Diagnostics.Tracing;
    using Win32;

    /// <summary>
    /// Static class containing methods relevant to logging telemetry.
    /// </summary>
    public class Telemetry
    {
        /// <summary>
        /// Path to the registry key that holds the telemetry settings for the group policy
        /// </summary>
        private const string TelemetryGroupPolicySettingsRegistryPath = @"Software\Policies\Microsoft\Windows\DataCollection";

        /// <summary>
        /// Path to the registry key that holds the telemetry settings for local user
        /// </summary>
        private const string TelemetryLocalSettingsRegistryPath = @"Software\Microsoft\Windows\CurrentVersion\Policies\DataCollection";

        /// <summary>
        /// Factory object to lazily initialize the singleton instance of this class
        /// </summary>
        private static Lazy<Telemetry> factory = new Lazy<Telemetry>(() => new Telemetry());

        /// <summary>
        /// The event source object used to write telemetry events
        /// </summary>
        private TelemetryEventSource eventSource;

        /// <summary>
        /// Flag indicating whether or not telemetry is allowed
        /// </summary>
        private bool isTelemetryAllowed;

        /// <summary>
        /// Correlation ID
        /// </summary>
        private Guid correlationId;

        /// <summary>
        /// The last used activity id
        /// </summary>
        private int lastUsedActivityId = 0;

        /// <summary>
        /// Map that holds the activity names used for different activity id's
        /// </summary>
        private Dictionary<int, string> activitiesMap = new Dictionary<int, string>();

        private Telemetry()
        {
        }

        public static Telemetry Instance
        {
            get
            {
                return factory.Value;
            }
        }

        private static string EntryAssemblyArgs
        {
            get
            {
                string[] args = Environment.GetCommandLineArgs();
                string commandLineArgs = string.Join(" ", args);
                return commandLineArgs;
            }
        }

        private static string EntryAssemblyVersion
        {
            get
            {
                string version = "unknown";
                try
                {
                    System.Reflection.Assembly assembly = System.Reflection.Assembly.GetEntryAssembly();
                    FileVersionInfo fvi = FileVersionInfo.GetVersionInfo(assembly.Location);
                    version = fvi.FileVersion;
                }
                catch (Exception)
                {
                }

                return version;
            }
        }

        private static string CurrentWindowsVersion
        {
            get
            {
                string versionString = "unknown";
                try
                {
                    using (var localMachineKey = RegistryKey.OpenBaseKey(
                        RegistryHive.LocalMachine,
                        Environment.Is64BitOperatingSystem ? RegistryView.Registry64 : RegistryView.Registry32))
                    {
                        using (var versionKey = localMachineKey.OpenSubKey("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"))
                        {
                            versionString = (string)versionKey.GetValue("BuildLabEx");
                        }
                    }
                }
                catch (Exception)
                {
                }

                return versionString;
            }
        }

        /// <summary>
        /// Initializes the singleton instance of this class for the specified provider.
        /// </summary>
        /// <param name="providerName">The name of the provider (event source) through which telemetry events will be written.</param>
        public void Initialize(string providerName)
        {
            this.eventSource = new TelemetryEventSource(providerName, TelemetryGroup.WindowsCoreTelemetry);
            this.isTelemetryAllowed = GetTelemetryLevel() != 0;
            this.correlationId = Guid.NewGuid();
        }

        /// <summary>
        /// Reports an exception that occurred during the execution of a activity, if an activity id is specified.
        /// </summary>
        /// <param name="exp">The exception that occurred</param>
        /// <param name="activityId">The ID of activity during which the exception occurred, generated by ReportActivityStartIfAllowed</param>
        public void ReportExceptionIfAllowed(Exception exp, int? activityId = null)
        {
            if (this.isTelemetryAllowed)
            {
                exp = exp.InnerException ?? exp;
                EventSourceOptions errorOption = TelemetryEventSource.TelemetryOptions();
                errorOption.Level = EventLevel.Error;

                this.eventSource.Write(
                    "ExceptionThrown",
                    errorOption,
                    new TelemetryEventData.ExceptionThrownEventData
                    {
                        CorrelationId = this.correlationId,
                        Time = DateTime.Now.Ticks / TimeSpan.TicksPerMillisecond,
                        ActivityId = activityId,
                        Message = exp.Message,
                        StackTrace = exp.StackTrace,
                    });
            }
        }

        /// <summary>
        /// Reports the start of a session
        /// </summary>
        public void ReportSessionStartIfAllowed()
        {
            if (this.isTelemetryAllowed)
            {
                EventSourceOptions telemetryOption = TelemetryEventSource.TelemetryOptions();

                this.eventSource.Write(
                    "SessionStarted",
                    telemetryOption,
                    new TelemetryEventData.SessionStartEventData
                    {
                        CorrelationId = this.correlationId,
                        Time = DateTime.Now.Ticks / TimeSpan.TicksPerMillisecond,
                        CommandLineArgs = EntryAssemblyArgs,
                        Version = EntryAssemblyVersion,
                        OsVersion = CurrentWindowsVersion
                    });
            }
        }

        /// <summary>
        /// Reports the end of the session
        /// </summary>
        /// <param name="exitCode">Exit code of the program</param>
        public void ReportSessionEndIfAllowed(int exitCode)
        {
            if (this.isTelemetryAllowed)
            {
                EventSourceOptions telemetryOption = TelemetryEventSource.TelemetryOptions();

                this.eventSource.Write(
                    "SessionEnded",
                    telemetryOption,
                    new TelemetryEventData.SessionEndEventData
                    {
                        CorrelationId = this.correlationId,
                        Time = DateTime.Now.Ticks / TimeSpan.TicksPerMillisecond,
                        ExitCode = exitCode
                    });
            }
        }

        /// <summary>
        /// Reports the execution of an activity (no start/end)
        /// </summary>
        /// <param name="activity">The name of the activity</param>
        /// <param name="activityData">The data of the activity</param>
        /// <typeparam name="T">the type of the activity data object</typeparam>
        public void ReportActivityIfAllowed<T>(string activity, T activityData)
        {
            if (this.isTelemetryAllowed)
            {
                EventSourceOptions telemetryOption = TelemetryEventSource.TelemetryOptions();

                Interlocked.Increment(ref this.lastUsedActivityId);

                this.eventSource.Write(
                    string.Format("Activity{0}", activity),
                    telemetryOption,
                    new TelemetryEventData.ActivityEventData<T>
                    {
                        CorrelationId = this.correlationId,
                        Time = DateTime.Now.Ticks / TimeSpan.TicksPerMillisecond,
                        ActivityId = this.lastUsedActivityId,
                        ActivityData = activityData
                    });
            }
        }

        /// <summary>
        /// Reports the start of an activity
        /// </summary>
        /// <param name="activity">The name of the activity</param>
        /// <param name="activityData">The data of the activity</param>
        /// <returns>A generated ID for the activity that was reported</returns>
        /// <typeparam name="T">the type of the activity data object</typeparam>
        public int ReportActivityStartIfAllowed<T>(string activity, T activityData)
        {
            if (this.isTelemetryAllowed)
            {
                EventSourceOptions telemetryOption = TelemetryEventSource.TelemetryOptions();

                Interlocked.Increment(ref this.lastUsedActivityId);
                this.activitiesMap[this.lastUsedActivityId] = activity;

                this.eventSource.Write(
                    string.Format("Activity{0}Started", activity),
                    telemetryOption,
                    new TelemetryEventData.ActivityEventData<T>
                    {
                        CorrelationId = this.correlationId,
                        Time = DateTime.Now.Ticks / TimeSpan.TicksPerMillisecond,
                        ActivityId = this.lastUsedActivityId,
                        ActivityData = activityData
                    });

                return this.lastUsedActivityId;
            }

            return 0;
        }

        /// <summary>
        /// Reports the end of the specified activity
        /// </summary>
        /// <param name="activityId">The ID of the activity, generated by ReportActivityStartIfAllowed</param>
        /// <param name="activityData">The data of the activity</param>
        /// <typeparam name="T">the type of the activity data object</typeparam>
        public void ReportActivityEndIfAllowed<T>(int activityId, T activityData)
        {
            if (this.isTelemetryAllowed)
            {
                EventSourceOptions telemetryOption = TelemetryEventSource.TelemetryOptions();

                string activity = this.activitiesMap[activityId];

                this.eventSource.Write(
                    string.Format("Activity{0}Ended", activity),
                    telemetryOption,
                    new TelemetryEventData.ActivityEventData<T>
                    {
                        CorrelationId = this.correlationId,
                        Time = DateTime.Now.Ticks / TimeSpan.TicksPerMillisecond,
                        ActivityId = activityId,
                        ActivityData = activityData
                    });
            }
        }

        /// <summary>
        /// See <see href="https://osgwiki.com/wiki/Telemetry_Levels#Registry_Key_Locations"/>
        /// </summary>
        /// <returns>the telemetry level (0 means not allowed)</returns>
        private static int GetTelemetryLevel()
        {
            int value = -1;
            using (RegistryKey key = Registry.LocalMachine.OpenSubKey(TelemetryGroupPolicySettingsRegistryPath, false))
            {
                if (key != null)
                {
                    value = (int)key.GetValue("AllowTelemetry", -1);
                    if (value == -1)
                    {
                        value = (int)key.GetValue("AllowTelemetry_PolicyManager", -1);
                    }
                }
            }

            if (value == -1)
            {
                using (RegistryKey key = Registry.LocalMachine.OpenSubKey(TelemetryLocalSettingsRegistryPath, false))
                {
                    if (key != null)
                    {
                        value = (int)key.GetValue("AllowTelemetry", -1);
                    }
                }
            }

            return value;
        }
    }
}